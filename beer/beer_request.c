#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

#include <msgpuck.h>

#include <beer/beer_mem.h>
#include <beer/beer_reply.h>
#include <beer/beer_stream.h>
#include <beer/beer_net.h>
#include <beer/beer_object.h>
#include <beer/beer_buf.h>
#include <beer/beer_proto.h>
#include <beer/beer_schema.h>

#include <beer/beer_request.h>

#include "beer_proto_internal.h"

struct beer_request *beer_request_init(struct beer_request *req) {
	int alloc = (req == NULL);
	if (req == NULL) {
		req = beer_mem_alloc(sizeof(struct beer_request));
		if (!req) return NULL;
	}
	memset(req, 0, sizeof(struct beer_request));
	req->limit = UINT32_MAX;
	req->alloc = alloc;
	return req;
};

void beer_request_free(struct beer_request *req) {
	if (req->key_object)
		beer_stream_free(req->key_object);
	req->key_object = NULL;
	if (req->tuple_object)
		beer_stream_free(req->tuple_object);
	req->tuple_object = NULL;
	if (req->alloc) beer_mem_free(req);
}

#define BEER_REQUEST_CUSTOM(NM, CNM)				\
struct beer_request *beer_request_##NM(struct beer_request *req) {	\
	req = beer_request_init(req);				\
	if (req) {						\
		req->hdr.type = BEER_OP_##CNM;			\
	}							\
	return req;						\
}

BEER_REQUEST_CUSTOM(select, SELECT);
BEER_REQUEST_CUSTOM(insert, INSERT);
BEER_REQUEST_CUSTOM(replace, REPLACE);
BEER_REQUEST_CUSTOM(update, UPDATE);
BEER_REQUEST_CUSTOM(delete, DELETE);
BEER_REQUEST_CUSTOM(call, CALL);
BEER_REQUEST_CUSTOM(call_16, CALL_16);
BEER_REQUEST_CUSTOM(auth, AUTH);
BEER_REQUEST_CUSTOM(eval, EVAL);
BEER_REQUEST_CUSTOM(upsert, UPSERT);
BEER_REQUEST_CUSTOM(ping, PING);

#undef BEER_REQUEST_CUSTOM

int beer_request_set_space(struct beer_request *req, uint32_t space)
{
	req->space_id = space;
	return 0;
}

int beer_request_set_index(struct beer_request *req, uint32_t index)
{
	req->index_id = index;
	return 0;
}

int beer_request_set_offset(struct beer_request *req, uint32_t offset)
{
	req->offset = offset;
	return 0;
}

int beer_request_set_limit(struct beer_request *req, uint32_t limit)
{
	req->limit = limit;
	return 0;
}

int
beer_request_set_iterator(struct beer_request *req, enum beer_iterator_t iterator)
{
	req->iterator = iterator;
	return 0;
}

int beer_request_set_index_base(struct beer_request *req, uint32_t index_base)
{
	req->index_base = index_base;
	return 0;
}

int beer_request_set_key(struct beer_request *req, struct beer_stream *s)
{
	req->key     = BEER_SBUF_DATA(s);
	req->key_end = req->key + BEER_SBUF_SIZE(s);
	return 0;
}

int beer_request_set_key_format(struct beer_request *req, const char *fmt, ...)
{
	if (req->key_object)
		beer_object_reset(req->key_object);
	else
		req->key_object = beer_object(NULL);
	if (!req->key_object)
		return -1;
	va_list args;
	va_start(args, fmt);
	ssize_t res = beer_object_vformat(req->key_object, fmt, args);
	va_end(args);
	if (res == -1)
		return -1;
	return beer_request_set_key(req, req->key_object);
}

int
beer_request_set_func(struct beer_request *req, const char *func,
		     uint32_t flen)
{
	if (!is_call(req->hdr.type))
		return -1;
	if (!func)
		return -1;
	req->key = func; req->key_end = req->key + flen;
	return 0;
}

int
beer_request_set_funcz(struct beer_request *req, const char *func)
{
	if (!is_call(req->hdr.type))
		return -1;
	if (!func)
		return -1;
	req->key = func; req->key_end = req->key + strlen(req->key);
	return 0;
}

int
beer_request_set_expr(struct beer_request *req, const char *expr,
		     uint32_t elen)
{
	if (req->hdr.type != BEER_OP_EVAL)
		return -1;
	if (!expr)
		return -1;
	req->key = expr; req->key_end = req->key + elen;
	return 0;
}

int
beer_request_set_exprz(struct beer_request *req, const char *expr)
{
	if (req->hdr.type != BEER_OP_EVAL)
		return -1;
	if (!expr)
		return -1;
	req->key = expr; req->key_end = req->key + strlen(req->key);
	return 0;
}

int
beer_request_set_ops(struct beer_request *req, struct beer_stream *s)
{
	if (req->hdr.type == BEER_OP_UPDATE) {
		req->tuple     = BEER_SBUF_DATA(s);
		req->tuple_end = req->tuple + BEER_SBUF_SIZE(s);
		return 0;
	} else if (req->hdr.type == BEER_OP_UPSERT) {
		req->key     = BEER_SBUF_DATA(s);
		req->key_end = req->key + BEER_SBUF_SIZE(s);
		return 0;
	}
	return -1;
}

int beer_request_set_tuple(struct beer_request *req, struct beer_stream *s)
{
	req->tuple     = BEER_SBUF_DATA(s);
	req->tuple_end = req->tuple + BEER_SBUF_SIZE(s);
	return 0;
}

int beer_request_set_tuple_format(struct beer_request *req, const char *fmt, ...)
{
	if (req->tuple_object)
		beer_object_reset(req->tuple_object);
	else
		req->tuple_object = beer_object(NULL);
	if (!req->tuple_object)
		return -1;
	va_list args;
	va_start(args, fmt);
	ssize_t res = beer_object_vformat(req->tuple_object, fmt, args);
	va_end(args);
	if (res == -1)
		return -1;
	return beer_request_set_tuple(req, req->tuple_object);
}

int
beer_request_writeout(struct beer_stream *s, struct beer_request *req,
		     uint64_t *sync) {
	enum beer_request_t tp = req->hdr.type;
	if (sync != NULL && *sync == INT64_MAX &&
	    (s->reqid & INT64_MAX) == INT64_MAX) {
		s->reqid = 0;
	}
	req->hdr.sync = s->reqid++;
	/* header */
	/* int (9) + 1 + sync + 1 + op */
	struct iovec v[10]; int v_sz = 0;
	char header[128];
	char *pos = header + 9;
	char *begin = pos;
	v[v_sz].iov_base = begin;
	v[v_sz++].iov_len  = 0;
	pos = mp_encode_map(pos, 2);              /* 1 */
	pos = mp_encode_uint(pos, BEER_CODE);      /* 1 */
	pos = mp_encode_uint(pos, req->hdr.type); /* 1 */
	pos = mp_encode_uint(pos, BEER_SYNC);      /* 1 */
	pos = mp_encode_uint(pos, req->hdr.sync); /* 9 */
	char *map = pos++;                        /* 1 */
	size_t nd = 0;
	if (tp < BEER_OP_CALL_16) {
		pos = mp_encode_uint(pos, BEER_SPACE);     /* 1 */
		pos = mp_encode_uint(pos, req->space_id); /* 5 */
		nd += 1;
	}
	if (req->index_id && (tp == BEER_OP_SELECT ||
			      tp == BEER_OP_UPDATE ||
			      tp == BEER_OP_DELETE)) {
		pos = mp_encode_uint(pos, BEER_INDEX);     /* 1 */
		pos = mp_encode_uint(pos, req->index_id); /* 5 */
		nd += 1;
	}
	if (tp == BEER_OP_SELECT) {
		pos = mp_encode_uint(pos, BEER_LIMIT);  /* 1 */
		pos = mp_encode_uint(pos, req->limit); /* 5 */
		nd += 1;
	}
	if (req->offset && tp == BEER_OP_SELECT) {
		pos = mp_encode_uint(pos, BEER_OFFSET);  /* 1 */
		pos = mp_encode_uint(pos, req->offset); /* 5 */
		nd += 1;
	}
	if (req->iterator && tp == BEER_OP_SELECT) {
		pos = mp_encode_uint(pos, BEER_ITERATOR);  /* 1 */
		pos = mp_encode_uint(pos, req->iterator); /* 1 */
		nd += 1;
	}
	if (req->key) {
		switch (tp) {
		case BEER_OP_EVAL:
			pos = mp_encode_uint(pos, BEER_EXPRESSION);          /* 1 */
			pos = mp_encode_strl(pos, req->key_end - req->key); /* 5 */
			break;
		case BEER_OP_CALL_16:
		case BEER_OP_CALL:
			pos = mp_encode_uint(pos, BEER_FUNCTION);            /* 1 */
			pos = mp_encode_strl(pos, req->key_end - req->key); /* 5 */
			break;
		case BEER_OP_SELECT:
		case BEER_OP_UPDATE:
		case BEER_OP_DELETE:
			pos = mp_encode_uint(pos, BEER_KEY); /* 1 */
			break;
		case BEER_OP_UPSERT:
			pos = mp_encode_uint(pos, BEER_OPS); /* 1 */
			break;
		default:
			return -1;
		}
		v[v_sz].iov_base  = begin;
		v[v_sz++].iov_len = pos - begin;
		begin = pos;
		v[v_sz].iov_base  = (void *)req->key;
		v[v_sz++].iov_len = req->key_end - req->key;
		nd += 1;
	}
	if (req->tuple) {
		pos = mp_encode_uint(pos, BEER_TUPLE); /* 1 */
		v[v_sz].iov_base  = begin;
		v[v_sz++].iov_len = pos - begin;
		begin = pos;
		v[v_sz].iov_base  = (void *)req->tuple;
		v[v_sz++].iov_len = req->tuple_end - req->tuple;
		nd += 1;
	}
	if (req->index_base && (tp == BEER_OP_UPDATE || tp == BEER_OP_UPSERT)) {
		pos = mp_encode_uint(pos, BEER_INDEX_BASE);  /* 1 */
		pos = mp_encode_uint(pos, req->index_base); /* 1 */
		nd += 1;
	}
	assert(mp_sizeof_map(nd) == 1);
	if (pos != begin) {
		v[v_sz].iov_base  = begin;
		v[v_sz++].iov_len = pos - begin;
	}
	mp_encode_map(map, nd);
	size_t plen = 0; nd = 0;
	for (int i = 1; i < v_sz; ++i) plen += v[i].iov_len;
	nd = mp_sizeof_luint32(plen);
	v[0].iov_base -= nd;
	v[0].iov_len  += nd;
	mp_encode_luint32(v[0].iov_base, plen);
	ssize_t rv = s->writev(s, v, v_sz);
	if (rv == -1)
		return -1;
	if (sync != NULL)
		*sync = req->hdr.sync;
	return 0;
}

int64_t
beer_request_compile(struct beer_stream *s, struct beer_request *req) {
	uint64_t sync = INT64_MAX;
	if (beer_request_writeout(s, req, &sync) == -1)
		return -1;
	return sync;
}
