#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>

#include <beer/beer_mem.h>
#include <beer/beer_reply.h>
#include <beer/beer_stream.h>
#include <beer/beer_buf.h>
#include <beer/beer_object.h>
#include <beer/beer_update.h>

#include "beer_proto_internal.h"

ssize_t
beer_update(struct beer_stream *s, uint32_t space, uint32_t index,
	   struct beer_stream *key, struct beer_stream *ops)
{
	if (beer_object_verify(key, MP_ARRAY))
		return -1;
	if (beer_object_verify(ops, MP_ARRAY))
		return -1;
	struct beer_iheader hdr;
	struct iovec v[6]; int v_sz = 6;
	char *data = NULL, *body_start = NULL;
	encode_header(&hdr, BEER_OP_UPDATE, s->reqid++);
	v[1].iov_base = (void *)hdr.header;
	v[1].iov_len  = hdr.end - hdr.header;
	char body[64]; body_start = body; data = body;

	data = mp_encode_map(data, 4);
	data = mp_encode_uint(data, BEER_SPACE);
	data = mp_encode_uint(data, space);
	data = mp_encode_uint(data, BEER_INDEX);
	data = mp_encode_uint(data, index);
	data = mp_encode_uint(data, BEER_KEY);
	v[2].iov_base = (void *)body_start;
	v[2].iov_len  = data - body_start;
	body_start = data;
	v[3].iov_base = BEER_SBUF_DATA(key);
	v[3].iov_len  = BEER_SBUF_SIZE(key);
	data = mp_encode_uint(data, BEER_TUPLE);
	v[4].iov_base = (void *)body_start;
	v[4].iov_len  = data - body_start;
	body_start = data;
	v[5].iov_base = BEER_SBUF_DATA(ops);
	v[5].iov_len  = BEER_SBUF_SIZE(ops);

	size_t package_len = 0;
	for (int i = 1; i < v_sz; ++i)
		package_len += v[i].iov_len;
	char len_prefix[9];
	char *len_end = mp_encode_luint32(len_prefix, package_len);
	v[0].iov_base = len_prefix;
	v[0].iov_len = len_end - len_prefix;
	return s->writev(s, v, v_sz);
}

ssize_t
beer_upsert(struct beer_stream *s, uint32_t space,
	   struct beer_stream *tuple, struct beer_stream *ops)
{
	if (beer_object_verify(tuple, MP_ARRAY))
		return -1;
	if (beer_object_verify(ops, MP_ARRAY))
		return -1;
	struct beer_iheader hdr;
	struct iovec v[6]; int v_sz = 6;
	char *data = NULL, *body_start = NULL;
	encode_header(&hdr, BEER_OP_UPSERT, s->reqid++);
	v[1].iov_base = (void *)hdr.header;
	v[1].iov_len  = hdr.end - hdr.header;
	char body[64]; body_start = body; data = body;

	data = mp_encode_map(data, 4);
	data = mp_encode_uint(data, BEER_SPACE);
	data = mp_encode_uint(data, space);
	data = mp_encode_uint(data, BEER_TUPLE);
	v[2].iov_base = (void *)body_start;
	v[2].iov_len  = data - body_start;
	body_start = data;
	v[3].iov_base = BEER_SBUF_DATA(tuple);
	v[3].iov_len  = BEER_SBUF_SIZE(tuple);
	data = mp_encode_uint(data, BEER_OPS);
	v[4].iov_base = (void *)body_start;
	v[4].iov_len  = data - body_start;
	body_start = data;
	v[5].iov_base = BEER_SBUF_DATA(ops);
	v[5].iov_len  = BEER_SBUF_SIZE(ops);

	size_t package_len = 0;
	for (int i = 1; i < v_sz; ++i)
		package_len += v[i].iov_len;
	char len_prefix[9];
	char *len_end = mp_encode_luint32(len_prefix, package_len);
	v[0].iov_base = len_prefix;
	v[0].iov_len = len_end - len_prefix;
	return s->writev(s, v, v_sz);
}

static ssize_t beer_update_op_len(char op) {
	switch (op) {
	case (BEER_UOP_ADDITION):
	case (BEER_UOP_SUBSTRACT):
	case (BEER_UOP_AND):
	case (BEER_UOP_XOR):
	case (BEER_UOP_OR):
	case (BEER_UOP_DELETE):
	case (BEER_UOP_INSERT):
	case (BEER_UOP_ASSIGN):
		return 3;
	case (BEER_UOP_SPLICE):
		return 5;
	default:
		return -1;
	}
}

struct beer_stream *beer_update_container(struct beer_stream *ops) {
	ops = beer_object(NULL);
	if (!ops) return NULL;
	beer_object_type(ops, BEER_SBO_SPARSE);
	if (beer_object_add_array(ops, 0) == -1) {
		beer_stream_free(ops);
		return NULL;
	}
	return ops;
}

int beer_update_container_close(struct beer_stream *ops) {
	struct beer_sbuf_object *opob = BEER_SOBJ_CAST(ops);
	opob->stack->size = ops->wrcnt - 1;
	beer_object_container_close(ops);
	return 0;
}

int beer_update_container_reset(struct beer_stream *ops) {
	beer_object_reset(ops);
	beer_object_type(ops, BEER_SBO_SPARSE);
	if (beer_object_add_array(ops, 0) == -1) {
		beer_stream_free(ops);
		return -1;
	}
	return 0;
}

static ssize_t
beer_update_op(struct beer_stream *ops, char op, uint32_t fieldno,
	      const char *opdata, size_t opdata_len) {
	struct iovec v[2]; size_t v_sz = 2;
	char body[64], *data; data = body;
	data = mp_encode_array(data, beer_update_op_len(op));
	data = mp_encode_str(data, &op, 1);
	data = mp_encode_uint(data, fieldno);
	v[0].iov_base = body;
	v[0].iov_len  = data - body;
	v[1].iov_base = (void *)opdata;
	v[1].iov_len  = opdata_len;

	return ops->writev(ops, v, v_sz);
}

ssize_t
beer_update_bit(struct beer_stream *ops, uint32_t fieldno, char op,
	       uint64_t value) {
	if (op != '&' && op != '^' && op != '|') return -1;
	char body[10], *data; data = body;
	data = mp_encode_uint(data, value);
	return beer_update_op(ops, op, fieldno, body, data - body);
}

ssize_t
beer_update_arith_int(struct beer_stream *ops, uint32_t fieldno, char op,
		     int64_t value) {
	if (op != '+' && op != '-') return -1;
	char body[10], *data; data = body;
	if (value >= 0)
		data = mp_encode_uint(data, value);
	else
		data = mp_encode_int(data, value);
	return beer_update_op(ops, op, fieldno, body, data - body);
}

ssize_t
beer_update_arith_float(struct beer_stream *ops, uint32_t fieldno, char op,
		       float value) {
	if (op != '+' && op != '-') return -1;
	char body[10], *data; data = body;
	data = mp_encode_float(data, value);
	return beer_update_op(ops, op, fieldno, body, data - body);
}

ssize_t
beer_update_arith_double(struct beer_stream *ops, uint32_t fieldno, char op,
		        double value) {
	if (op != '+' && op != '-') return -1;
	char body[10], *data; data = body;
	data = mp_encode_double(data, value);
	return beer_update_op(ops, op, fieldno, body, data - body);
}

ssize_t
beer_update_delete(struct beer_stream *ops, uint32_t fieldno,
		  uint32_t fieldcount) {
	char body[10], *data; data = body;
	data = mp_encode_uint(data, fieldcount);
	return beer_update_op(ops, '#', fieldno, body, data - body);
}

ssize_t
beer_update_insert(struct beer_stream *ops, uint32_t fieldno,
		  struct beer_stream *val) {
	if (beer_object_verify(val, -1))
		return -1;
	return beer_update_op(ops, '!', fieldno, BEER_SBUF_DATA(val),
			     BEER_SBUF_SIZE(val));
}

ssize_t
beer_update_assign(struct beer_stream *ops, uint32_t fieldno,
		  struct beer_stream *val) {
	if (beer_object_verify(val, -1))
		return -1;
	return beer_update_op(ops, '=', fieldno, BEER_SBUF_DATA(val),
			     BEER_SBUF_SIZE(val));
}

ssize_t
beer_update_splice(struct beer_stream *ops, uint32_t fieldno,
		  uint32_t position, uint32_t offset,
		  const char *buffer, size_t buffer_len) {
	size_t buf_size = mp_sizeof_uint(position) +
		          mp_sizeof_uint(offset) +
			  mp_sizeof_str(buffer_len);
	char *buf = beer_mem_alloc(buf_size), *data = NULL;
	if (!buf) return -1;
	data = buf;
	data = mp_encode_uint(data, position);
	data = mp_encode_uint(data, offset);
	data = mp_encode_str(data, buffer, buffer_len);
	ssize_t retval = beer_update_op(ops, ':', fieldno, buf, buf_size);
	beer_mem_free(buf);
	return retval;
}
