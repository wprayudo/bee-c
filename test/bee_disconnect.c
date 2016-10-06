#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <bee/bee.h>
#include <beer/beer_net.h>
#include <beer/beer_opt.h>

int main() {
	const char * uri = "localhost:3301";
	struct beer_stream * beer = beer_net(NULL); // Allocating stream
	beer_set(beer, BEER_OPT_URI, uri); // Setting URI
	beer_set(beer, BEER_OPT_SEND_BUF, 0); // Disable buffering for send
	beer_set(beer, BEER_OPT_RECV_BUF, 0); // Disable buffering for recv
	beer_connect(beer);

	while (1) {
		if (beer_error(beer) != BEER_EOK) {
			printf("disconnected %s\n", beer_strerror(beer));
			beer_connect(beer);
		}
		printf("next_request\n");
		beer_ping(beer); // Send ping request
		struct beer_reply * reply = beer_reply_init(NULL); // Initialize reply
		beer->read_reply(beer, reply); // Read reply from server
		beer_reply_free(reply); // Free reply
		usleep(1000000);
	}

	beer_stream_free(beer); // Close connection and free stream object
}
