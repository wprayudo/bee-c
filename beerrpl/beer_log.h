#ifndef BEER_LOG_H_INCLUDED
#define BEER_LOG_H_INCLUDED

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

#define BEER_LOG_MAGIC_XLOG "XLOG\n"
#define BEER_LOG_MAGIC_SNAP "SNAP\n"
#define BEER_LOG_VERSION "0.11\n"

enum beer_log_error {
	BEER_LOG_EOK,
	BEER_LOG_EFAIL,
	BEER_LOG_EMEMORY,
	BEER_LOG_ETYPE,
	BEER_LOG_EVERSION,
	BEER_LOG_ECORRUPT,
	BEER_LOG_ESYSTEM,
	BEER_LOG_LAST
};

struct beer_log_header_v11 {
	uint32_t crc32_hdr;
	uint64_t lsn;
	double tm;
	uint32_t len;
	uint32_t crc32_data;
} __attribute__((packed));

struct beer_log_row_v11 {
	uint16_t tag;
	uint64_t cookie;
	uint16_t op;
} __attribute__((packed));

struct beer_log_row_snap_v11 {
	uint16_t tag;
	uint64_t cookie;
	uint32_t space;
	uint32_t tuple_size;
	uint32_t data_size;
} __attribute__((packed));

enum beer_log_type {
	BEER_LOG_NONE,
	BEER_LOG_XLOG,
	BEER_LOG_SNAPSHOT
};

union beer_log_value {
	struct beer_request r;
	struct beer_tuple t;
};

struct beer_log_row {
	struct beer_log_header_v11 hdr;
	struct beer_log_row_v11 row;
	struct beer_log_row_snap_v11 row_snap;
	union beer_log_value *value;
};

struct beer_log {
	enum beer_log_type type;
	FILE *fd;
	off_t current_offset;
	off_t offset;
	int (*read)(struct beer_log *l, char **buf, uint32_t *size);
	int (*process)(struct beer_log *l, char *buf, uint32_t size,
		       union beer_log_value *value);
	struct beer_log_row current;
	union beer_log_value current_value;
	enum beer_log_error error;
	int errno_;
};

extern const uint32_t beer_log_marker_v11;
extern const uint32_t beer_log_marker_eof_v11;

enum beer_log_type beer_log_guess(char *file);

enum beer_log_error
beer_log_open(struct beer_log *l, char *file, enum beer_log_type type);
int beer_log_seek(struct beer_log *l, off_t offset);
void beer_log_close(struct beer_log *l);

struct beer_log_row *beer_log_next(struct beer_log *l);
struct beer_log_row *beer_log_next_to(struct beer_log *l, union beer_log_value *value);

enum beer_log_error beer_log_error(struct beer_log *l);
char *beer_log_strerror(struct beer_log *l);
int beer_log_errno(struct beer_log *l);

#endif /* BEER_LOG_H_INCLUDED */
