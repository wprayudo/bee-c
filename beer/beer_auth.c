#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>

#include <beer/beer_reply.h>
#include <beer/beer_stream.h>
#include <beer/beer_ping.h>

#include <beer/beer_net.h>

#include <sha1.h>
#include <base64.h>

#include "beer_proto_internal.h"

static inline void
beer_xor(unsigned char *to, const unsigned char *left,
	const unsigned char *right, uint32_t len)
{
	const uint8_t *end = to + len;
	while (to < end)
		*to++= *left++ ^ *right++;
}
static inline void
beer_scramble_prepare(void *out, const void *salt, const void *pass, int plen)
{
	unsigned char hash1[BEER_SCRAMBLE_SIZE];
	unsigned char hash2[BEER_SCRAMBLE_SIZE];
	SHA1_CTX ctx;

	SHA1Init(&ctx);
	SHA1Update(&ctx, (const unsigned char *) pass, plen);
	SHA1Final(hash1, &ctx);

	SHA1Init(&ctx);
	SHA1Update(&ctx, hash1, BEER_SCRAMBLE_SIZE);
	SHA1Final(hash2, &ctx);

	SHA1Init(&ctx);
	SHA1Update(&ctx, (const unsigned char *) salt, BEER_SCRAMBLE_SIZE);
	SHA1Update(&ctx, hash2, BEER_SCRAMBLE_SIZE);
	SHA1Final((unsigned char *) out, &ctx);

	beer_xor((unsigned char *) out, hash1, (const unsigned char *) out,
	       BEER_SCRAMBLE_SIZE);
}


ssize_t
beer_auth_raw(struct beer_stream *s, const char *user, int ulen,
	     const char *pass, int plen, const char *base64_salt)
{
	struct beer_iheader hdr;
	struct iovec v[6]; int v_sz = 5;
	char *data = NULL, *body_start = NULL;
	int guest = !user || (ulen == 5 && !strncmp(user, "guest", 5));
	if (guest) {
		user = "guest";
		ulen = 5;
	}
	encode_header(&hdr, BEER_OP_AUTH, s->reqid++);
	v[1].iov_base = (void *)hdr.header;
	v[1].iov_len  = hdr.end - hdr.header;
	char body[64]; data = body; body_start = data;

	data = mp_encode_map(data, 2);
	data = mp_encode_uint(data, BEER_USERNAME);
	data = mp_encode_strl(data, ulen);
	v[2].iov_base = body_start;
	v[2].iov_len  = data - body_start;
	v[3].iov_base = (void *)user;
	v[3].iov_len  = ulen;
	body_start = data;
	data = mp_encode_uint(data, BEER_TUPLE);
	if (!guest) {
		data = mp_encode_array(data, 2);
		data = mp_encode_str(data, "chap-sha1", strlen("chap-sha1"));
		data = mp_encode_strl(data, BEER_SCRAMBLE_SIZE);
		char salt[64], scramble[BEER_SCRAMBLE_SIZE];
		base64_decode(base64_salt, BEER_SALT_SIZE, salt, 64);
		beer_scramble_prepare(scramble, salt, pass, plen);
		v[5].iov_base = scramble;
		v[5].iov_len  = BEER_SCRAMBLE_SIZE;
		v_sz++;
	} else {
		data = mp_encode_array(data, 0);
	}
	v[4].iov_base = body_start;
	v[4].iov_len  = data - body_start;

	size_t package_len = 0;
	for (int i = 1; i < v_sz; ++i) {
		package_len += v[i].iov_len;
	}
	char len_prefix[9];
	char *len_end = mp_encode_luint32(len_prefix, package_len);
	v[0].iov_base = len_prefix;
	v[0].iov_len = len_end - len_prefix;
	return s->writev(s, v, v_sz);
}

ssize_t
beer_auth(struct beer_stream *s, const char *user, int ulen,
	 const char *pass, int plen)
{
	return beer_auth_raw(s, user, ulen, pass, plen,
			    BEER_SNET_CAST(s)->greeting + BEER_VERSION_SIZE);
}

ssize_t
beer_deauth(struct beer_stream *s)
{
	return beer_auth(s, NULL, 0, NULL, 0);
}
