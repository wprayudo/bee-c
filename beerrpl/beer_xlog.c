
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
#include <beer_xlog.h>

static void beer_xlog_free(struct beer_stream *s) {
	struct beer_stream_xlog *sx = BEER_SXLOG_CAST(s);
	beer_log_close(&sx->log);
	beer_mem_free(s->data);
	s->data = NULL;
}

static int
beer_xlog_request(struct beer_stream *s, struct beer_request *r)
{
	struct beer_stream_xlog *sx = BEER_SXLOG_CAST(s);

	struct beer_log_row *row =
		beer_log_next_to(&sx->log, (union beer_log_value*)r);

	if (row == NULL && beer_log_error(&sx->log) == BEER_LOG_EOK)
		return 1;

	return (row) ? 0: -1;
}

/*
 * beer_xlog()
 *
 * create and initialize xlog stream;
 *
 * s - stream pointer, maybe NULL
 * 
 * if stream pointer is NULL, then new stream will be created. 
 *
 * returns stream pointer, or NULL on error.
*/
struct beer_stream *beer_xlog(struct beer_stream *s)
{
	int allocated = s == NULL;
	s = beer_stream_init(s);
	if (s == NULL)
		return NULL;
	/* allocating stream data */
	s->data = beer_mem_alloc(sizeof(struct beer_stream_xlog));
	if (s->data == NULL) {
		if (allocated)
			beer_stream_free(s);
		return NULL;
	}
	memset(s->data, 0, sizeof(struct beer_stream_xlog));
	/* initializing interfaces */
	s->read = NULL;
	s->read_request = beer_xlog_request;
	s->read_reply = NULL;
	s->read_tuple = NULL;
	s->write = NULL;
	s->writev = NULL;
	s->free = beer_xlog_free;
	/* initializing internal data */
	return s;
}

/*
 * beer_xlog_open()
 *
 * open xlog file and associate it with stream;
 *
 * s - xlog stream pointer
 * 
 * returns 0 on success, or -1 on error.
*/
int beer_xlog_open(struct beer_stream *s, char *file) {
	struct beer_stream_xlog *sx = BEER_SXLOG_CAST(s);
	return beer_log_open(&sx->log, file, BEER_LOG_XLOG);
}

/*
 * beer_xlog_close()
 *
 * close xlog stream; 
 *
 * s - xlog stream pointer
 * 
 * returns 0 on success, or -1 on error.
*/
void beer_xlog_close(struct beer_stream *s) {
	struct beer_stream_xlog *sx = BEER_SXLOG_CAST(s);
	beer_log_close(&sx->log);
}

/*
 * beer_xlog_error()
 *
 * get stream error status;
 *
 * s - xlog stream pointer
*/
enum beer_log_error beer_xlog_error(struct beer_stream *s) {
	return BEER_SXLOG_CAST(s)->log.error;
}

/*
 * beer_xlog_strerror()
 *
 * get stream error status description string;
 *
 * s - xlog stream pointer
*/
char *beer_xlog_strerror(struct beer_stream *s) {
	return beer_log_strerror(&BEER_SXLOG_CAST(s)->log);
}

/*
 * beer_xlog_errno()
 *
 * get saved errno;
 *
 * s - xlog stream pointer
*/
int beer_xlog_errno(struct beer_stream *s) {
	return BEER_SXLOG_CAST(s)->log.errno_;
}
