#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <bee/bee.h>
#include <beer/beer_net.h>
#include <beer/beer_opt.h>

#include "common.h"

int64_t CallBeeFunction(struct beer_stream* stream, const char* name,
			       const char* format, ...)
{
	struct beer_request* request   = beer_request_call_16(NULL);
	struct beer_stream*  arguments = beer_object(NULL);

	va_list list;
	va_start(list, format);
	beer_object_vformat(arguments, format, list);
	va_end(list);

	beer_request_set_funcz(request, name);
	beer_request_set_tuple(request, arguments);

	int64_t result = beer_request_compile(stream, request);

	beer_stream_free(arguments);
	beer_request_free(request);
	beer_flush(stream);

	return result;
}

int main() {
	const char * uri = "localhost:3301";
	struct beer_stream * beer = beer_net(NULL); // Allocating stream
	beer_set(beer, BEER_OPT_URI, uri); // Setting URI
	beer_set(beer, BEER_OPT_SEND_BUF, 0); // Disable buffering for send
	beer_set(beer, BEER_OPT_RECV_BUF, 0); // Disable buffering for recv
	beer_connect(beer);
	CallBeeFunction(beer, "test_1","[{%s%d%s%s%s%u%s%d%s%d%s%u%s%u%s%llu%s%u}]",
			      "kind",        110,
			      "name",        "FastForward",
			      "number",      2621,
			      "slot",        0,
			      "flavor",      7,
			      "source",      2321040,
			      "destination", 2321,
			      "tag",         0,
			      "network",     2501);
	struct beer_reply *reply = beer_reply_init(NULL); // Initialize reply
	beer->read_reply(beer, reply); // Read reply from server

	check_rbytes(reply, NULL, 0);

	beer_reply_free(reply); // Free reply
}
