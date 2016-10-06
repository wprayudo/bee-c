#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

#include <msgpuck.h>

#include <beer/beer_reply.h>
#include <beer/beer_stream.h>
#include <beer/beer_buf.h>
#include <beer/beer_object.h>
#include <beer/beer_insert.h>

#include "beer_proto_internal.h"

static ssize_t
beer_store_base(struct beer_stream *s, uint32_t space, struct beer_stream *tuple,
	       enum beer_request_t op)
{
	if (beer_object_verify(tuple, MP_ARRAY))
		return -1;
	struct beer_iheader hdr;
	struct iovec v[4]; int v_sz = 4;
	char *data = NULL;
	encode_header(&hdr, op, s->reqid++);
	v[1].iov_base = (void *)hdr.header;
	v[1].iov_len  = hdr.end - hdr.header;
	char body[64]; data = body;

	data = mp_encode_map(data, 2);
	data = mp_encode_uint(data, BEER_SPACE);
	data = mp_encode_uint(data, space);
	data = mp_encode_uint(data, BEER_TUPLE);
	v[2].iov_base = body;
	v[2].iov_len  = data - body;
	v[3].iov_base = BEER_SBUF_DATA(tuple);
	v[3].iov_len  = BEER_SBUF_SIZE(tuple);

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
beer_insert(struct beer_stream *s, uint32_t space, struct beer_stream *tuple)
{
	return beer_store_base(s, space, tuple, BEER_OP_INSERT);
}

ssize_t
beer_replace(struct beer_stream *s, uint32_t space, struct beer_stream *tuple)
{
	return beer_store_base(s, space, tuple, BEER_OP_REPLACE);
}
