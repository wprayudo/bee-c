#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <sys/uio.h>

#include <msgpuck/msgpuck.h>

#include <beer/beer_net.h>

void hexDump (char *desc, void *addr, int len) {
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char*)addr;

	if (desc != NULL)
		printf ("%s:\n", desc);
	for (i = 0; i < len; i++) {
		if ((i % 16) == 0) {
			if (i != 0) printf ("  %s\n", buff);
			printf ("  %04x ", i);
		}
		printf (" %02x", pc[i]);
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}
	while ((i % 16) != 0) {
		printf ("   ");
		i++;
	}
	printf ("  %s\n", buff);
}

int main() {
	struct beer_stream *s = beer_net(NULL);
	struct beer_reply r; beer_reply_init(&r);
	assert(beer_set(s, BEER_OPT_URI, "myamlya:1234@/tmp/taran_tool.sock\0") != -1);
	assert(beer_connect(s) != -1);
	assert(beer_authenticate(s) != -1);
	assert(beer_reload_schema(s) != -1);
	struct beer_stream *empty = beer_object(NULL);
	beer_object_add_array(empty, 0);

	int rc = beer_call_16(s, "fiber.time", 10, empty);
	rc += beer_eval(s, "fiber.time", 10, empty);
	hexDump("fiber_time (call+eval)", BEER_SNET_CAST(s)->sbuf.buf, rc);
	rc = beer_flush(s);
	printf("%d\n", rc);
	printf("%d\n", s->read_reply(s, &r));
	hexDump("response call", (void *)r.buf, r.buf_size); beer_reply_free(&r);
	printf("%d\n", s->read_reply(s, &r));
	hexDump("response eval", (void *)r.buf, r.buf_size); beer_reply_free(&r);

	struct beer_stream *arr = beer_object(NULL);
	beer_object_add_array(arr, 3);
	beer_object_add_int(arr, 1);
	beer_object_add_int(arr, 2);
	beer_object_add_int(arr, 3);

	rc  = beer_insert(s, 512, arr);
	rc += beer_replace(s, 512, arr);
	hexDump("insert+replace [1,2,3]", BEER_SNET_CAST(s)->sbuf.buf, rc);
	rc = beer_flush(s);
	printf("%d\n", rc);
	printf("%d\n", s->read_reply(s, &r));
	hexDump("response insert", (void *)r.buf, r.buf_size); beer_reply_free(&r);
	printf("%d\n", s->read_reply(s, &r));
	hexDump("response replace", (void *)r.buf, r.buf_size); beer_reply_free(&r);

	rc  = beer_ping(s);
	hexDump("ping", BEER_SNET_CAST(s)->sbuf.buf, rc);
	rc = beer_flush(s);
	printf("%d\n", rc);
	printf("%d\n", s->read_reply(s, &r));
	hexDump("response ping", (void *)r.buf, r.buf_size); beer_reply_free(&r);

	struct beer_stream *key = beer_object(NULL);
	beer_object_add_array(key, 1);
	beer_object_add_int(key, 1);

	rc  = beer_select(s, 512, 0, UINT32_MAX, 0, 0, key);
	hexDump("select [1]", BEER_SNET_CAST(s)->sbuf.buf, rc);
	rc = beer_flush(s);
	printf("%d\n", rc);
	printf("%d\n", s->read_reply(s, &r));
	hexDump("response select", (void *)r.buf, r.buf_size); beer_reply_free(&r);

	rc  = beer_delete(s, 512, 0, key);
	hexDump("delete [1]", BEER_SNET_CAST(s)->sbuf.buf, rc);
	rc = beer_flush(s);
	printf("%d\n", rc);
	printf("%d\n", s->read_reply(s, &r));
	hexDump("response delete", (void *)r.buf, r.buf_size); beer_reply_free(&r);

	rc  = beer_auth(s, "myamlya", 7, "1234", 4);
	rc += beer_deauth(s);
	rc += beer_auth(s, "guest", 5, NULL, 0);
	hexDump("auth No1", BEER_SNET_CAST(s)->sbuf.buf, rc);
	rc = beer_flush(s);
	printf("%d\n", rc);
	printf("%d\n", s->read_reply(s, &r));
	hexDump("response auth", (void *)r.buf, r.buf_size); beer_reply_free(&r);
	printf("%d\n", s->read_reply(s, &r));
	hexDump("response deauth", (void *)r.buf, r.buf_size); beer_reply_free(&r);
	printf("%d\n", s->read_reply(s, &r));
	hexDump("response auth deauth", (void *)r.buf, r.buf_size); beer_reply_free(&r);

	beer_stream_free(arr);
	arr = beer_object(NULL);
	beer_object_add_array(arr, 8);
	beer_object_add_int(arr, 1);
	beer_object_add_int(arr, 2);
	beer_object_add_float(arr, 3.0);
	beer_object_add_double(arr, 4.0);
	beer_object_add_int(arr, 31);
	beer_object_add_int(arr, 6);
	beer_object_add_strz(arr, "hello, brian");
	beer_object_add_int(arr, 1111);
	hexDump("arr", BEER_SBUF_DATA(arr), BEER_SBUF_SIZE(arr));

	rc = beer_replace(s, 512, arr);
	hexDump("replace [1, 2, 3.0F, 4.0D, 5, 6, \"hello, brian\", 1111]", BEER_SNET_CAST(s)->sbuf.buf, rc);
	rc = beer_flush(s);
	printf("%d\n", rc);
	printf("%d\n", s->read_reply(s, &r));
	hexDump("response replace", (void *)r.buf, r.buf_size); beer_reply_free(&r);

	struct beer_stream *ops = beer_buf(NULL);
	assert(beer_update_arith_int(ops, 1, '+', 10) != -1);
	assert(beer_update_arith_float(ops, 2, '+', 3.2) != -1);
	assert(beer_update_arith_double(ops, 3, '+', 7.8) != -1);
	assert(beer_update_bit(ops, 4, '&', 0x10001) != -1);
	assert(beer_update_assign(ops, 8, arr) != -1);
	assert(beer_update_splice(ops, 6, 7, 5, "master", 6) != -1);
	assert(beer_update_insert(ops, 8, arr) != -1);
	
	rc = beer_update(s, 512, 0, key, ops);
	hexDump("update", BEER_SNET_CAST(s)->sbuf.buf, rc);
	rc = beer_flush(s);
	printf("%d\n", rc);
	printf("%d\n", s->read_reply(s, &r));
	hexDump("response update", (void *)r.buf, r.buf_size); beer_reply_free(&r);

	return 0;
}
