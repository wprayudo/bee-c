
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
#include <stdint.h>
#include <string.h>

#include <beer/beer_mem.h>
#include <beer/beer_proto.h>
#include <beer/beer_reply.h>
#include <beer/beer_stream.h>
#include <beer/beer_buf.h>

static void beer_buf_free(struct beer_stream *s) {
	struct beer_stream_buf *sb = BEER_SBUF_CAST(s);
	if (!sb->as && sb->data)
		beer_mem_free(sb->data);
	if (sb->free)
		sb->free(s);
	beer_mem_free(s->data);
	s->data = NULL;
}

static ssize_t
beer_buf_read(struct beer_stream *s, char *buf, size_t size) {
	struct beer_stream_buf *sb = BEER_SBUF_CAST(s);
	if (sb->data == NULL)
		return 0;
	if (sb->size == sb->rdoff)
		return 0;
	size_t avail = sb->size - sb->rdoff;
	if (size > avail)
		size = avail;
	memcpy(sb->data + sb->rdoff, buf, size);
	sb->rdoff += size;
	return size;
}

static char* beer_buf_resize(struct beer_stream *s, size_t size) {
	struct beer_stream_buf *sb = BEER_SBUF_CAST(s);
	size_t off = sb->size;
	size_t nsize = off + size;
	char *nd = realloc(sb->data, nsize);
	if (nd == NULL) {
		beer_mem_free(sb->data);
		return NULL;
	}
	sb->data = nd;
	sb->alloc = nsize;
	return sb->data + off;
}

static ssize_t
beer_buf_write(struct beer_stream *s, const char *buf, size_t size) {
	if (BEER_SBUF_CAST(s)->as == 1) return -1;
	char *p = BEER_SBUF_CAST(s)->resize(s, size);
	if (p == NULL)
		return -1;
	memcpy(p, buf, size);
	BEER_SBUF_CAST(s)->size += size;
	s->wrcnt++;
	return size;
}

static ssize_t
beer_buf_writev(struct beer_stream *s, struct iovec *iov, int count) {
	if (BEER_SBUF_CAST(s)->as == 1) return -1;
	size_t size = 0;
	int i;
	for (i = 0 ; i < count ; i++)
		size += iov[i].iov_len;
	char *p = BEER_SBUF_CAST(s)->resize(s, size);
	if (p == NULL)
		return -1;
	for (i = 0 ; i < count ; i++) {
		memcpy(p, iov[i].iov_base, iov[i].iov_len);
		p += iov[i].iov_len;
	}
	BEER_SBUF_CAST(s)->size += size;
	s->wrcnt++;
	return size;
}

static int
beer_buf_reply(struct beer_stream *s, struct beer_reply *r) {
	struct beer_stream_buf *sb = BEER_SBUF_CAST(s);
	if (sb->data == NULL)
		return -1;
	if (sb->size == sb->rdoff)
		return 1;
	size_t off = 0;
	int rc = beer_reply(r, sb->data + sb->rdoff, sb->size - sb->rdoff, &off);
	if (rc == 0)
		sb->rdoff += off;
	return rc;
}

struct beer_stream *beer_buf(struct beer_stream *s) {
	int allocated = s == NULL;
	s = beer_stream_init(s);
	if (s == NULL)
		return NULL;
	/* allocating stream data */
	s->data = beer_mem_alloc(sizeof(struct beer_stream_buf));
	if (s->data == NULL) {
		if (allocated)
			beer_stream_free(s);
		return NULL;
	}
	/* initializing interfaces */
	s->read       = beer_buf_read;
	s->read_reply = beer_buf_reply;
	s->write      = beer_buf_write;
	s->writev     = beer_buf_writev;
	s->free       = beer_buf_free;
	/* initializing internal data */
	struct beer_stream_buf *sb = BEER_SBUF_CAST(s);
	sb->rdoff   = 0;
	sb->size    = 0;
	sb->alloc   = 0;
	sb->data    = NULL;
	sb->resize  = beer_buf_resize;
	sb->free    = NULL;
	sb->subdata = NULL;
	sb->as      = 0;
	return s;
}

struct beer_stream *beer_buf_as(struct beer_stream *s, char *buf, size_t buf_len)
{
	if (s == NULL) {
		s = beer_buf(s);
		if (s == NULL)
			return NULL;
	}
	struct beer_stream_buf *sb = BEER_SBUF_CAST(s);

	sb->data = buf;
	sb->size = buf_len;
	sb->alloc = buf_len;
	sb->as = 1;

	return s;
}
