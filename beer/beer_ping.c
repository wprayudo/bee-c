#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>

#include <beer/beer_reply.h>
#include <beer/beer_stream.h>
#include <beer/beer_ping.h>

#include "beer_proto_internal.h"

ssize_t
beer_ping(struct beer_stream *s)
{
	struct beer_iheader hdr;
	struct iovec v[3]; int v_sz = 3;
	char *data = NULL;
	encode_header(&hdr, BEER_OP_PING, s->reqid++);
	v[1].iov_base = (void *)hdr.header;
	v[1].iov_len  = hdr.end - hdr.header;
	char body[2]; data = body;

	data = mp_encode_map(data, 0);
	v[2].iov_base = body;
	v[2].iov_len  = data - body;

	size_t package_len = 0;
	for (int i = 1; i < v_sz; ++i)
		package_len += v[i].iov_len;
	char len_prefix[9];
	char *len_end = mp_encode_luint32(len_prefix, package_len);
	v[0].iov_base = len_prefix;
	v[0].iov_len = len_end - len_prefix;
	return s->writev(s, v, v_sz);
}
