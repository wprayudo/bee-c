#include "test.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <bee/bee.h>

#include <beer/beer_net.h>
#include <beer/beer_opt.h>

#include "common.h"

#define header() note("*** %s: prep ***", __func__)
#define footer() note("*** %s: done ***", __func__)

static int
test_connect_unix(char *uri) {
	plan(3);
	header();

	struct beer_stream *beer = NULL; beer = beer_net(NULL);
	isnt(beer, NULL, "Check connection creation");
	isnt(beer_set(beer, BEER_OPT_URI, uri), -1, "Setting URI");
	isnt(beer_connect(beer), -1, "Connecting");
//	isnt(beer_authenticate(beer), -1, "Authenticating");

	beer_stream_free(beer);

	footer();
	return check_plan();
}

static int
test_ping(char *uri) {
	plan(7);
	header();

	struct beer_stream *beer = NULL; beer = beer_net(NULL);
	isnt(beer, NULL, "Check connection creation");
	isnt(beer_set(beer, BEER_OPT_URI, uri), -1, "Setting URI");
	isnt(beer_connect(beer), -1, "Connecting");
//	isnt(beer_authenticate(beer), -1, "Authenticating");

	isnt(beer_ping(beer), -1, "Create ping");
	isnt(beer_flush(beer), -1, "Send to server");

	struct beer_reply reply;
	beer_reply_init(&reply);
	isnt(beer->read_reply(beer, &reply), -1, "Read reply from server");
	is  (reply.error, NULL, "Check error absence");

	beer_reply_free(&reply);
	beer_stream_free(beer);

	footer();
	return check_plan();
}

static int
test_auth_call(char *uri) {
	plan(23);
	header();

	const char bb1[]="\x83\x00\xce\x00\x00\x00\x00\x01\xcf\x00\x00\x00\x00\x00"
			 "\x00\x00\x02\x05\xce\x00\x00\x00\x37\x81\x30\xdd\x00\x00"
			 "\x00\x01\x91\xa5\x67\x75\x65\x73\x74";
	size_t bb1_len = sizeof(bb1) - 1;
	const char bb2[]="\x83\x00\xce\x00\x00\x00\x00\x01\xcf\x00\x00\x00\x00\x00"
			 "\x00\x00\x04\x05\xce\x00\x00\x00\x37\x81\x30\xdd\x00\x00"
			 "\x00\x01\xa4\x74\x65\x73\x74";
	size_t bb2_len = sizeof(bb2) - 1;

	struct beer_stream *args = NULL; args = beer_object(NULL);
	isnt(args, NULL, "Check object creation");
	isnt(beer_object_format(args, "[]"), -1, "check object filling");

	struct beer_reply reply;
	struct beer_stream *beer = NULL; beer = beer_net(NULL);
	isnt(beer, NULL, "Check connection creation");
	isnt(beer_set(beer, BEER_OPT_URI, uri), -1, "Setting URI");
	isnt(beer_connect(beer), -1, "Connecting");
//	isnt(beer_authenticate(beer), -1, "Authenticating");

	isnt(beer_deauth(beer), -1, "Create deauth");
	isnt(beer_flush(beer), -1, "Send to server");

	beer_reply_init(&reply);
	isnt(beer->read_reply(beer, &reply), -1, "Read reply from server");
	is  (reply.error, NULL, "Check error absence");
	beer_reply_free(&reply);

	isnt(beer_call_16(beer, "test_4", 6, args), -1, "Create call request");
	isnt(beer_flush(beer), -1, "Send to server");

	beer_reply_init(&reply);
	isnt(beer->read_reply(beer, &reply), -1, "Read reply from server");
	is  (reply.error, NULL, "Check error absence");
	is  (check_rbytes(&reply, bb1, bb1_len), 0, "Check response");
	beer_reply_free(&reply);

	isnt(beer_auth(beer, "test", 4, "test", 4), -1, "Create auth");
	isnt(beer_flush(beer), -1, "Send to server");

	beer_reply_init(&reply);
	isnt(beer->read_reply(beer, &reply), -1, "Read reply from server");
	is  (reply.error, NULL, "Check error absence");
	beer_reply_free(&reply);

	isnt(beer_eval(beer, "return test_4()", 15, args), -1, "Create eval "
							     "request");
	isnt(beer_flush(beer), -1, "Send to server");

	beer_reply_init(&reply);
	isnt(beer->read_reply(beer, &reply), -1, "Read reply from server");
	is  (reply.error, NULL, "Check error absence");
	is  (check_rbytes(&reply, bb2, bb2_len), 0, "Check response");

	beer_stream_free(args);
	beer_reply_free(&reply);
	beer_stream_free(beer);

	footer();
	return check_plan();
}

static int
test_insert_replace_delete(char *uri) {
	plan(186);
	header();

	struct beer_stream *beer = NULL; beer = beer_net(NULL);
	isnt(beer, NULL, "Check connection creation");
	isnt(beer_set(beer, BEER_OPT_URI, uri), -1, "Setting URI");
	isnt(beer_connect(beer), -1, "Connecting");
//	isnt(beer_authenticate(beer), -1, "Authenticating");
	beer_stream_reqid(beer, 0);

	for (int i = 0; i < 10; ++i) {
		char ex[128] = {0};
		size_t ex_len = snprintf(ex, 128, "examplestr %d %d", i, i*i);

		struct beer_stream *val = beer_object(NULL);
		beer_object_format(val, "[%d%d%.*s]", i, i + 10, ex_len, ex);
		beer_insert(beer, 512, val);

		beer_stream_free(val);
	}

	isnt(beer_flush(beer), -1, "Send package to server");

	struct beer_iter it;
	beer_iter_reply(&it, beer);
	while (beer_next(&it)) {
		struct beer_reply *r = BEER_IREPLY_PTR(&it);
		uint32_t i = r->sync, str_len = 0;
		char ex[128] = {0};
		size_t ex_len = snprintf(ex, 128, "examplestr %d %d", i, i*i);
		isnt(r->data, NULL, "check that we get answer");
		const char *data = r->data;
		is  (mp_typeof(*data), MP_ARRAY, "Check array");
		is  (mp_decode_array(&data), 1, "Check array, again");
		is  (mp_decode_array(&data), 3, "And again (another)");
		ok  (mp_typeof(*data) == MP_UINT &&
		     mp_decode_uint(&data) == i &&
		     mp_typeof(*data) == MP_UINT &&
		     mp_decode_uint(&data) == i + 10 &&
		     mp_typeof(*data) == MP_STR &&
		     strncmp(mp_decode_str(&data, &str_len), ex, ex_len) == 0,
		     "Check fields");
	}

	beer_stream_reqid(beer, 0);
	for (int i = 0; i < 5; ++i) {
		char ex[128] = {0};
		size_t ex_len;
		ex_len = snprintf(ex, 128, "anotherexamplestr %d %d", i, i*i);

		struct beer_stream *val = beer_object(NULL);
		beer_object_format(val, "[%d%d%.*s]", i, i + 5, ex_len, ex);
		beer_replace(beer, 512, val);

		beer_stream_free(val);
	}

	isnt(beer_flush(beer), -1, "Send package to server");

	beer_iter_reply(&it, beer);
	while (beer_next(&it)) {
		struct beer_reply *r = BEER_IREPLY_PTR(&it);
		uint32_t i = r->sync, str_len = 0;
		char ex[128] = {0};
		size_t ex_len;
		ex_len = snprintf(ex, 128, "anotherexamplestr %d %d", i, i*i);
		isnt(r->data, NULL, "check that we get answer");
		const char *data = r->data;
		is  (mp_typeof(*data), MP_ARRAY, "Check array");
		is  (mp_decode_array(&data), 1, "Check array, again");
		is  (mp_decode_array(&data), 3, "And again (another)");
		ok  (mp_typeof(*data) == MP_UINT &&
		     mp_decode_uint(&data) == i &&
		     mp_typeof(*data) == MP_UINT &&
		     mp_decode_uint(&data) == i + 5 &&
		     mp_typeof(*data) == MP_STR &&
		     strncmp(mp_decode_str(&data, &str_len), ex, ex_len) == 0,
		     "Check fields");
	}

	struct beer_stream *key = NULL; key = beer_object(NULL);
	isnt(key, NULL, "Check object creation");
	is  (beer_object_add_array(key, 0), 1, "Create key");
	beer_select(beer, 512, 0, UINT32_MAX, 0, 0, key);
	beer_stream_free(key);

	beer_flush(beer);

	struct beer_reply reply; beer_reply_init(&reply);
	isnt(beer->read_reply(beer, &reply), -1, "Read reply");
	const char *data = reply.data;

	is  (mp_typeof(*data), MP_ARRAY, "Check array");
	uint32_t vsz = mp_decode_array(&data);
	is  (vsz, 10, "Check array, again");

	uint32_t arrsz = vsz;
	uint32_t str_len = 0;

	while (arrsz-- > 0) {
		is  (mp_decode_array(&data), 3, "And again (another)");
		is  (mp_typeof(*data), MP_UINT, "check int");
		uint32_t sz = mp_decode_uint(&data);
		is  (mp_typeof(*data), MP_UINT, "check int");
		uint32_t sz_z = sz + 10; if (sz < 5) sz_z -= 5;
		uint32_t vsz = mp_decode_uint(&data);
		is  (vsz, sz_z, "check int val");
		char ex[128] = {0};
		size_t ex_len = 0;
		if (sz < 5)
			ex_len = snprintf(ex, 128, "anotherexamplestr %d %d",
					  sz, sz*sz);
		else
			ex_len = snprintf(ex, 128, "examplestr %d %d", sz, sz*sz);
		ok  (mp_typeof(*data) == MP_STR &&
		     strncmp(mp_decode_str(&data, &str_len), ex, ex_len) == 0,
		     "Check str");
	}

	beer_reply_free(&reply);
	beer_stream_reqid(beer, 0);

	for (int i = 0; i < 10; ++i) {
		struct beer_stream *key = beer_object(NULL);
		beer_object_format(key, "[%d]", i);
		beer_delete(beer, 512, 0, key);

		beer_stream_free(key);
	}

	isnt(beer_flush(beer), -1, "Send package to server");

	beer_iter_reply(&it, beer);
	while (beer_next(&it)) {
		struct beer_reply *r = BEER_IREPLY_PTR(&it);
		uint32_t i = r->sync, str_len = 0, nlen = (i < 5 ? i + 5 : i + 10);
		char ex[128] = {0};
		size_t ex_len = 0;
		if (i < 5)
			ex_len = snprintf(ex, 128, "anotherexamplestr %d %d",
					  i, i*i);
		else
			ex_len = snprintf(ex, 128, "examplestr %d %d", i, i*i);
		isnt(r->data, NULL, "check that we get answer");
		const char *data = r->data;
		is  (mp_typeof(*data), MP_ARRAY, "Check array");
		is  (mp_decode_array(&data), 1, "Check array, again");
		is  (mp_decode_array(&data), 3, "And again (another)");
		ok  (mp_typeof(*data) == MP_UINT &&
		     mp_decode_uint(&data) == i &&
		     mp_typeof(*data) == MP_UINT &&
		     mp_decode_uint(&data) == nlen &&
		     mp_typeof(*data) == MP_STR &&
		     strncmp(mp_decode_str(&data, &str_len), ex, ex_len) == 0,
		     "Check fields");
	}
	beer_stream_free(beer);

	footer();
	return check_plan();
}

int main() {
	plan(4);

	char uri[128] = {0};
	snprintf(uri, 128, "%s%s", "test:test@", getenv("PRIMARY_PORT"));

	test_connect_unix(uri);
	test_ping(uri);
	test_auth_call(uri);
	test_insert_replace_delete(uri);

	return check_plan();
}
