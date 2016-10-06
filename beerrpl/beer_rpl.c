
/*
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <bee.h>
#include <beer_net.h>
#include <beer_io.h>
#include <beer_log.h>
#include <beer_rpl.h>

static const uint32_t beer_rpl_version = 11;

static void beer_rpl_free(struct beer_stream *s) {
	struct beer_stream_rpl *sr = BEER_RPL_CAST(s);
	if (sr->net) {
		/* network stream should not be free'd here */
		sr->net = NULL;
	}
	beer_mem_free(s->data);
}

static ssize_t
beer_rpl_recv_cb(struct beer_stream *s, char *buf, ssize_t size) {
	struct beer_stream_net *sn = BEER_SNET_CAST(s);
	return beer_io_recv(sn, buf, size);
}

static int
beer_rpl_request(struct beer_stream *s, struct beer_request *r)
{
	struct beer_stream_rpl *sr = BEER_RPL_CAST(s);
	struct beer_stream_net *sn = BEER_SNET_CAST(sr->net);
	/* fetching header */
	if (beer_io_recv(sn, (char*)&sr->hdr, sizeof(sr->hdr)) == -1)
		return -1;
	/* fetching row header */
	if (beer_io_recv(sn, (char*)&sr->row, sizeof(sr->row)) == -1)
		return -1;
	/* preparing pseudo iproto header */
	struct beer_header hdr_iproto;
	hdr_iproto.type = sr->row.op;
	hdr_iproto.len = sr->hdr.len - sizeof(struct beer_log_row_v11);
	hdr_iproto.reqid = 0;
	/* deserializing operation */
	if (beer_request_from(r, (beer_request_t)beer_rpl_recv_cb,
			     sr->net,
			     &hdr_iproto) == -1)
		return -1;
	return 0;
}

/*
 * beer_rpl()
 *
 * create and initialize replication stream;
 *
 * s - stream pointer, maybe NULL
 * 
 * if stream pointer is NULL, then new stream will be created. 
 *
 * returns stream pointer, or NULL on error.
*/
struct beer_stream *beer_rpl(struct beer_stream *s)
{
	int allocated = s == NULL;
	s = beer_stream_init(s);
	if (s == NULL)
		return NULL;
	/* allocating stream data */
	s->data = beer_mem_alloc(sizeof(struct beer_stream_rpl));
	if (s->data == NULL)
		goto error;
	memset(s->data, 0, sizeof(struct beer_stream_rpl));
	/* initializing interfaces */
	s->read = NULL;
	s->read_request = beer_rpl_request;
	s->read_reply = NULL;
	s->read_tuple = NULL;
	s->write = NULL;
	s->writev = NULL;
	s->free = beer_rpl_free;
	/* initializing internal data */
	struct beer_stream_rpl *sr = BEER_RPL_CAST(s);
	sr->net = NULL;
	return s;
error:
	if (s->data) {
		beer_mem_free(s->data);
		s->data = NULL;
	}
	if (allocated)
		beer_stream_free(s);
	return NULL;
}

/*
 * beer_rpl_open()
 *
 * connect to a server and initialize handshake;
 *
 * s   - replication stream pointer
 * lsn - start lsn 
 *
 * network stream must be properly initialized before
 * this function called (see tbeer_rpl_net, beer_set).
 * 
 * returns 0 on success, or -1 on error.
*/
int beer_rpl_open(struct beer_stream *s, uint64_t lsn)
{
	struct beer_stream_rpl *sr = BEER_RPL_CAST(s);
	/* intializing connection */
	if (beer_init(sr->net) == -1)
		return -1;
	if (beer_connect(sr->net) == -1)
		return -1;
	/* sending initial lsn */
	struct beer_stream_net *sn = BEER_SNET_CAST(sr->net);
	if (beer_io_send_raw(sn, (char*)&lsn, sizeof(lsn), 1) == -1)
		return -1;
	/* reading and checking version */
	uint32_t version = 0;
	if (beer_io_recv_raw(sn, (char*)&version, sizeof(version), 1) == -1)
		return -1;
	if (version != beer_rpl_version)
		return -1;
	return 0;
}

/*
 * beer_rpl_close()
 *
 * close a connection; 
 *
 * s - replication stream pointer
 * 
 * returns 0 on success, or -1 on error.
*/
void beer_rpl_close(struct beer_stream *s) {
	struct beer_stream_rpl *sr = BEER_RPL_CAST(s);
	if (sr->net)
		beer_close(s);
}

/*
 * beer_rpl_attach()
 *
 * attach network stream (beer_stream_net object);
 *
 * s - replication stream pointer
*/
void beer_rpl_attach(struct beer_stream *s, struct beer_stream *net) {
	BEER_RPL_CAST(s)->net = net;
}
