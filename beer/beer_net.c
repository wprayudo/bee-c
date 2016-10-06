
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/uio.h>

#include <uri.h>

#include <beer/beer_proto.h>
#include <beer/beer_reply.h>
#include <beer/beer_stream.h>
#include <beer/beer_object.h>
#include <beer/beer_mem.h>
#include <beer/beer_schema.h>
#include <beer/beer_select.h>
#include <beer/beer_iter.h>
#include <beer/beer_auth.h>

#include <beer/beer_net.h>
#include <beer/beer_io.h>

#include "pmatomic.h"

static void beer_net_free(struct beer_stream *s) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	beer_io_close(sn);
	beer_mem_free(sn->greeting);
	beer_iob_free(&sn->sbuf);
	beer_iob_free(&sn->rbuf);
	beer_opt_free(&sn->opt);
	beer_schema_free(sn->schema);
	beer_mem_free(sn->schema);
	beer_mem_free(s->data);
	s->data = NULL;
}

static ssize_t
beer_net_read(struct beer_stream *s, char *buf, size_t size) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	/* read doesn't touches wrcnt */
	return beer_io_recv(sn, buf, size);
}

static ssize_t
beer_net_write(struct beer_stream *s, const char *buf, size_t size) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	ssize_t rc = beer_io_send(sn, buf, size);
	if (rc != -1)
		pm_atomic_fetch_add(&s->wrcnt, 1);
	return rc;
}

static ssize_t
beer_net_writev(struct beer_stream *s, struct iovec *iov, int count) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	ssize_t rc = beer_io_sendv(sn, iov, count);
	if (rc != -1)
		pm_atomic_fetch_add(&s->wrcnt, 1);
	return rc;
}

static ssize_t
beer_net_recv_cb(struct beer_stream *s, char *buf, ssize_t size) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	return beer_io_recv(sn, buf, size);
}

static int
beer_net_reply(struct beer_stream *s, struct beer_reply *r) {
	if (pm_atomic_load(&s->wrcnt) == 0)
		return 1;
	pm_atomic_fetch_sub(&s->wrcnt, 1);
	return beer_reply_from(r, (beer_reply_t)beer_net_recv_cb, s);
}

struct beer_stream *beer_net(struct beer_stream *s) {
	s = beer_stream_init(s);
	if (s == NULL)
		return NULL;
	/* allocating stream data */
	s->data = beer_mem_alloc(sizeof(struct beer_stream_net));
	if (s->data == NULL) {
		beer_stream_free(s);
		return NULL;
	}
	memset(s->data, 0, sizeof(struct beer_stream_net));
	/* initializing interfaces */
	s->read = beer_net_read;
	s->read_reply = beer_net_reply;
	s->write = beer_net_write;
	s->writev = beer_net_writev;
	s->free = beer_net_free;
	/* initializing internal data */
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	sn->fd = -1;
	sn->greeting = beer_mem_alloc(BEER_GREETING_SIZE);
	if (sn->greeting == NULL) {
		beer_stream_free(s);
	}
	if (beer_opt_init(&sn->opt) == -1) {
		beer_stream_free(s);
	}
	return s;
}

int beer_set(struct beer_stream *s, int opt, ...) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	va_list args;
	va_start(args, opt);
	sn->error = beer_opt_set(&sn->opt, opt, args);
	va_end(args);
	return (sn->error == BEER_EOK) ? 0 : -1;
}

int beer_init(struct beer_stream *s) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	if ((sn->schema = beer_schema_new(NULL)) == NULL) {
		sn->error = BEER_EMEMORY;
		return -1;
	}
	if (beer_iob_init(&sn->sbuf, sn->opt.send_buf, sn->opt.send_cb,
		sn->opt.send_cbv, sn->opt.send_cb_arg) == -1) {
		sn->error = BEER_EMEMORY;
		return -1;
	}
	if (beer_iob_init(&sn->rbuf, sn->opt.recv_buf, sn->opt.recv_cb, NULL,
		sn->opt.recv_cb_arg) == -1) {
		sn->error = BEER_EMEMORY;
		return -1;
	}
	sn->inited = 1;
	return 0;
}

int beer_reload_schema(struct beer_stream *s)
{
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	if (!sn->connected || pm_atomic_load(&s->wrcnt) != 0)
		return -1;
	uint64_t oldsync = beer_stream_reqid(s, 127);
	beer_get_space(s);
	beer_get_index(s);
	beer_stream_reqid(s, oldsync);
	beer_flush(s);
	struct beer_iter it; beer_iter_reply(&it, s);
	struct beer_reply bkp; beer_reply_init(&bkp);
	int sloaded = 0;
	while (beer_next(&it)) {
		struct beer_reply *r = BEER_IREPLY_PTR(&it);
		switch (r->sync) {
		case(127):
			if (r->error)
				goto error;
			beer_schema_add_spaces(sn->schema, r);
			sloaded += 1;
			break;
		case(128):
			if (r->error)
				goto error;
			if (!(sloaded & 1)) {
				memcpy(&bkp, r, sizeof(struct beer_reply));
				r->buf = NULL;
				break;
			}
			sloaded += 2;
			beer_schema_add_indexes(sn->schema, r);
			break;
		default:
			goto error;
		}
	}
	if (bkp.buf) {
		beer_schema_add_indexes(sn->schema, &bkp);
		sloaded += 2;
		beer_reply_free(&bkp);
	}
	if (sloaded != 3) goto error;

	beer_iter_free(&it);
	return 0;
error:
	beer_iter_free(&it);
	return -1;
}

static int
beer_authenticate(struct beer_stream *s)
{
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	if (!sn->connected || pm_atomic_load(&s->wrcnt) != 0)
		return -1;
	struct uri *uri = sn->opt.uri;
	beer_auth(s, uri->login, uri->login_len, uri->password,
		 uri->password_len);
	beer_flush(s);
	struct beer_reply rep;
	beer_reply_init(&rep);
	if (s->read_reply(s, &rep) == -1)
		return -1;
	if (rep.error != NULL) {
		if (BEER_REPLY_ERR(&rep) == BEER_ER_PASSWORD_MISMATCH)
			sn->error = BEER_ELOGIN;
		return -1;
	}
	beer_reply_free(&rep);
	beer_reload_schema(s);
	return 0;
}

int beer_connect(struct beer_stream *s)
{
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	if (!sn->inited) beer_init(s);
	if (sn->connected)
		beer_close(s);
	sn->error = beer_io_connect(sn);
	if (sn->error != BEER_EOK)
		return -1;
	if (s->read(s, sn->greeting, BEER_GREETING_SIZE) == -1 ||
	    sn->error != BEER_EOK)
		return -1;
	if (sn->opt.uri->login && sn->opt.uri->password)
		if (beer_authenticate(s) == -1)
			return -1;
	return 0;
}

void beer_close(struct beer_stream *s) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	beer_iob_clear(&sn->sbuf);
	beer_iob_clear(&sn->rbuf);
	beer_io_close(sn);
	s->wrcnt = 0;
	s->reqid = 0;
}

ssize_t beer_flush(struct beer_stream *s) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	return beer_io_flush(sn);
}

int beer_fd(struct beer_stream *s) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	return sn->fd;
}

enum beer_error beer_error(struct beer_stream *s) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	return sn->error;
}

/* must be in sync with enum beer_error */

struct beer_error_desc {
	enum beer_error type;
	char *desc;
};

static struct beer_error_desc beer_error_list[] =
{
	{ BEER_EOK,      "ok"                       },
	{ BEER_EFAIL,    "fail"                     },
	{ BEER_EMEMORY,  "memory allocation failed" },
	{ BEER_ESYSTEM,  "system error"             },
	{ BEER_EBIG,     "buffer is too big"        },
	{ BEER_ESIZE,    "bad buffer size"          },
	{ BEER_ERESOLVE, "gethostbyname(2) failed"  },
	{ BEER_ETMOUT,   "operation timeout"        },
	{ BEER_EBADVAL,  "bad argument"             },
	{ BEER_ELOGIN,   "failed to login"          },
	{ BEER_LAST,      NULL                      }
};

char *beer_strerror(struct beer_stream *s)
{
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	if (sn->error == BEER_ESYSTEM) {
		static char msg[256];
		snprintf(msg, sizeof(msg), "%s (errno: %d)",
			 strerror(sn->errno_), sn->errno_);
		return msg;
	}
	return beer_error_list[(int)sn->error].desc;
}

int beer_errno(struct beer_stream *s) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	return sn->errno_;
}

int beer_get_spaceno(struct beer_stream *s, const char *space,
		    size_t space_len)
{
	struct beer_schema *sch = (BEER_SNET_CAST(s))->schema;
	return beer_schema_stosid(sch, space, space_len);
}

int beer_get_indexno(struct beer_stream *s, int spaceno, const char *index,
		    size_t index_len)
{
	struct beer_schema *sch = BEER_SNET_CAST(s)->schema;
	return beer_schema_stoiid(sch, spaceno, index, index_len);
}
