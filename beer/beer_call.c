#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

#include <msgpuck.h>

#include <beer/beer_reply.h>
#include <beer/beer_stream.h>
#include <beer/beer_buf.h>
#include <beer/beer_object.h>
#include <beer/beer_call.h>

#include "beer_proto_internal.h"

static ssize_t
beer_rpc_base(struct beer_stream *s, const char *proc, size_t proc_len,
	     struct beer_stream *args, enum beer_request_t op)
{
	if (!proc || proc_len == 0)
		return -1;
	if (beer_object_verify(args, MP_ARRAY))
		return -1;
	uint32_t fld = (is_call(op) ? BEER_FUNCTION : BEER_EXPRESSION);
	struct beer_iheader hdr;
	struct iovec v[6]; int v_sz = 6;
	char *data = NULL, *body_start = NULL;
	encode_header(&hdr, op, s->reqid++);
	v[1].iov_base = (void *)hdr.header;
	v[1].iov_len  = hdr.end - hdr.header;
	char body[64]; body_start = body; data = body;

	data = mp_encode_map(data, 2);
	data = mp_encode_uint(data, fld);
	data = mp_encode_strl(data, proc_len);
	v[2].iov_base = body_start;
	v[2].iov_len  = data - body_start;
	v[3].iov_base = (void *)proc;
	v[3].iov_len  = proc_len;
	body_start = data;
	data = mp_encode_uint(data, BEER_TUPLE);
	v[4].iov_base = body_start;
	v[4].iov_len  = data - body_start;
	v[5].iov_base = BEER_SBUF_DATA(args);
	v[5].iov_len  = BEER_SBUF_SIZE(args);

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
beer_call(struct beer_stream *s, const char *proc, size_t proc_len,
	 struct beer_stream *args)
{
	return beer_rpc_base(s, proc, proc_len, args, BEER_OP_CALL);
}

ssize_t
beer_call_16(struct beer_stream *s, const char *proc, size_t proc_len,
	    struct beer_stream *args)
{
	return beer_rpc_base(s, proc, proc_len, args, BEER_OP_CALL_16);
}

ssize_t
beer_eval(struct beer_stream *s, const char *proc, size_t proc_len,
	 struct beer_stream *args)
{
	return beer_rpc_base(s, proc, proc_len, args, BEER_OP_EVAL);
}
