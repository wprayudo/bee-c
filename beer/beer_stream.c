
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
#include <stdint.h>
#include <string.h>

#include <sys/types.h>

#include <beer/beer_mem.h>
#include <beer/beer_proto.h>
#include <beer/beer_reply.h>
#include <beer/beer_stream.h>

uint32_t
beer_stream_reqid(struct beer_stream *s, uint32_t reqid)
{
	uint32_t old = s->reqid;
	s->reqid = reqid;
	return old;
}

struct beer_stream*
beer_stream_init(struct beer_stream *s)
{
	int alloc = (s == NULL);
	if (alloc) {
		s = beer_mem_alloc(sizeof(struct beer_stream));
		if (s == NULL)
			return NULL;
	}
	memset(s, 0, sizeof(struct beer_stream));
	s->alloc = alloc;
	return s;
}

void beer_stream_free(struct beer_stream *s) {
	if (s == NULL)
		return;
	if (s->free)
		s->free(s);
	if (s->alloc)
		beer_mem_free(s);
}
