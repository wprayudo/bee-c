
/*
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <third_party/crc32.h>

#include <bee.h>
#include <beer_log.h>

enum beer_log_type beer_log_guess(char *file) {
	if (file == NULL)
		return BEER_LOG_XLOG;
	char *ext = strrchr(file, '.');
	if (ext == NULL)
		return BEER_LOG_NONE;
	if (strcasecmp(ext, ".snap") == 0)
		return BEER_LOG_SNAPSHOT;
	if (strcasecmp(ext, ".xlog") == 0)
		return BEER_LOG_XLOG;
	return BEER_LOG_NONE;
}

inline static int
beer_log_seterr(struct beer_log *l, enum beer_log_error e) {
	l->error = e;
	if (e == BEER_LOG_ESYSTEM)
		l->errno_ = errno;
	return -1;
}

const uint32_t beer_log_marker_v11 = 0xba0babed;
const uint32_t beer_log_marker_eof_v11 = 0x10adab1e;

inline static int
beer_log_eof(struct beer_log *l, char *data) {
	uint32_t marker = 0;
	if (data)
		beer_mem_free(data);
	/* checking eof condition */
	if (ftello(l->fd) == l->offset + sizeof(beer_log_marker_eof_v11)) {
		fseeko(l->fd, l->offset, SEEK_SET);
		if (fread(&marker, sizeof(marker), 1, l->fd) != 1)
			return beer_log_seterr(l, BEER_LOG_ESYSTEM);
		else
		if (marker != beer_log_marker_eof_v11)
			return beer_log_seterr(l, BEER_LOG_ECORRUPT);
		l->offset = ftello(l->fd);
	}
	/* eof */
	return 1;
}

static int beer_log_read(struct beer_log *l, char **buf, uint32_t *size)
{
	/* current record offset (before marker) */
	l->current_offset = ftello(l->fd);

	/* reading marker */
	char *data = NULL;
	uint32_t marker = 0;
	if (fread(&marker, sizeof(marker), 1, l->fd) != 1)
		return beer_log_eof(l, data);

	/* seeking for marker if necessary */
	while (marker != beer_log_marker_v11) {
		int c = fgetc(l->fd);
		if (c == EOF)
			return beer_log_eof(l, data);
		marker = marker >> 8 | ((uint32_t) c & 0xff) <<
			 (sizeof(marker) * 8 - 8);
	}

	/* reading header */
	if (fread(&l->current.hdr, sizeof(l->current.hdr), 1, l->fd) != 1)
		return beer_log_eof(l, data);

	/* updating offset */
	l->offset = ftello(l->fd);

	/* checking header crc, starting from lsn */
	uint32_t crc32_hdr =
		crc32c(0, (unsigned char*)&l->current.hdr + sizeof(uint32_t),
		       sizeof(struct beer_log_header_v11) -
		       sizeof(uint32_t));
	if (crc32_hdr != l->current.hdr.crc32_hdr)
		return beer_log_seterr(l, BEER_LOG_ECORRUPT);

	/* allocating memory and reading data */
	data = beer_mem_alloc(l->current.hdr.len);
	if (data == NULL)
		return beer_log_seterr(l, BEER_LOG_EMEMORY);
	if (fread(data, l->current.hdr.len, 1, l->fd) != 1)
		return beer_log_eof(l, data);

	/* checking data crc */
	uint32_t crc32_data = crc32c(0, (unsigned char*)data, l->current.hdr.len);
	if (crc32_data != l->current.hdr.crc32_data) {
		beer_mem_free(data);
		return beer_log_seterr(l, BEER_LOG_ECORRUPT);
	}

	*buf = data;
	*size = l->current.hdr.len;
	return 0;
}

static int
beer_log_process_xlog(struct beer_log *l, char *buf, uint32_t size,
		     union beer_log_value *value)
{
	(void)size;
	/* copying row data */
	memcpy(&l->current.row, buf, sizeof(l->current.row));

	/* preparing pseudo iproto header */
	struct beer_header hdr_iproto;
	hdr_iproto.type = l->current.row.op;
	hdr_iproto.len = l->current.hdr.len - sizeof(l->current.row);
	hdr_iproto.reqid = 0;

	/* deserializing operation */
	beer_request_init(&value->r);
	size_t off = 0;
	int rc = beer_request(&value->r,
			     buf + sizeof(l->current.row),
			     l->current.hdr.len - sizeof(l->current.row),
			     &off,
			     &hdr_iproto);

	/* in case of not completed request or parsing error */
	if (rc != 0)
		return beer_log_seterr(l, BEER_LOG_ECORRUPT);
	return 0;
}

static int
beer_log_process_snapshot(struct beer_log *l, char *buf, uint32_t size,
		         union beer_log_value *value)
{
	(void)size;

	/* freeing previously allocated tuple */
	beer_tuple_free(&value->t);

	/* copying snapshot row data */
	memcpy(&l->current.row_snap, buf, sizeof(l->current.row_snap));

	/* reading and validating tuple */
	struct beer_tuple *tu =
		beer_tuple_set_as(&value->t, buf + sizeof(l->current.row_snap),
				 l->current.row_snap.data_size,
				 l->current.row_snap.tuple_size);
	if (tu == NULL)
		return beer_log_seterr(l, BEER_LOG_ECORRUPT);

	return (tu) ? 0 : -1;
}

struct beer_log_row*
beer_log_next_to(struct beer_log *l, union beer_log_value *value) {
	char *buf = NULL;
	uint32_t size = 0;
	int rc = l->read(l, &buf, &size);
	if (rc != 0)
		return NULL;
	rc = l->process(l, buf, size, value);
	if (rc != 0) {
		beer_mem_free(buf);
		return NULL;
	}
	if (l->type == BEER_LOG_XLOG) {
		beer_request_setorigin(&value->r, buf, size);
	} else {
		beer_mem_free(buf);
	}
	return &l->current;
}

struct beer_log_row *beer_log_next(struct beer_log *l) {
	return beer_log_next_to(l, &l->current_value);
}

inline static int
beer_log_open_err(struct beer_log *l, enum beer_log_error e) {
	beer_log_seterr(l, e);
	beer_log_close(l);
	return -1;
}

enum beer_log_error
beer_log_open(struct beer_log *l, char *file, enum beer_log_type type)
{
	char filetype[32];
	char version[32];
	char *rc, *magic = "\0";
	l->type = type;
	/* trying to open file */
	if (file) {
		l->fd = fopen(file, "r");
		if (l->fd == NULL)
			return beer_log_open_err(l, BEER_LOG_ESYSTEM);
	} else {
		l->fd = stdin;
	}
	/* reading xlog filetype */
	rc = fgets(filetype, sizeof(filetype), l->fd);
	if (rc == NULL)
		return beer_log_open_err(l, BEER_LOG_ESYSTEM);
	/* reading log version */
	rc = fgets(version, sizeof(version), l->fd);
	if (rc == NULL)
		return beer_log_open_err(l, BEER_LOG_ESYSTEM);
	/* checking file type and setting read/process
	 * interfaces */
	l->read = beer_log_read;
	switch (type) {
	case BEER_LOG_XLOG:
		magic = BEER_LOG_MAGIC_XLOG;
		l->process = beer_log_process_xlog;
		break;
	case BEER_LOG_SNAPSHOT:
		magic = BEER_LOG_MAGIC_SNAP;
		l->process = beer_log_process_snapshot;
		break;
	case BEER_LOG_NONE:
		break;
	}
	if (strcmp(filetype, magic))
		return beer_log_open_err(l, BEER_LOG_ETYPE);
	/* checking version */
	if (strcmp(version, BEER_LOG_VERSION))
		return beer_log_open_err(l, BEER_LOG_EVERSION);
	for (;;) {
		char buf[256];
		rc = fgets(buf, sizeof(buf), l->fd);
		if (rc == NULL)
			return beer_log_open_err(l, BEER_LOG_EFAIL);
		if (strcmp(rc, "\n") == 0 || strcmp(rc, "\r\n") == 0)
			break;
	}
	/* getting current offset */
	l->offset = ftello(l->fd);
	l->current_offset = 0;
	memset(&l->current_value, 0, sizeof(l->current_value));
	return 0;
}

void beer_log_close(struct beer_log *l) {
	if (l->fd && l->fd != stdin)
		fclose(l->fd);
	l->fd = NULL;
}

int beer_log_seek(struct beer_log *l, off_t offset) 
{
	l->offset = offset;
	return fseeko(l->fd, offset, SEEK_SET);
}

enum beer_log_error beer_log_error(struct beer_log *l) {
	return l->error;
}

struct beer_log_error_desc {
	enum beer_log_error type;
	char *desc;
};

static struct beer_log_error_desc beer_log_error_list[] = 
{
	{ BEER_LOG_EOK,      "ok"                                },
	{ BEER_LOG_EFAIL,    "fail"                              },
	{ BEER_LOG_EMEMORY,  "memory allocation failed"          },
	{ BEER_LOG_ETYPE,    "file type mismatch"                },
	{ BEER_LOG_EVERSION, "file version mismatch"             },
	{ BEER_LOG_ECORRUPT, "file crc failed or bad eof marker" },
	{ BEER_LOG_ESYSTEM,  "system error"                      },
	{ BEER_LOG_LAST,      NULL                               }
};

char *beer_log_strerror(struct beer_log *l) {
	if (l->error == BEER_LOG_ESYSTEM) {
		static char msg[256];
		snprintf(msg, sizeof(msg), "%s (errno: %d)",
			 strerror(l->errno_),
			 l->errno_);
		return msg;
	}
	return beer_log_error_list[(int)l->error].desc;
}

int beer_log_errno(struct beer_log *l) {
	return l->errno_;
}
