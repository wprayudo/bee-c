
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
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/uio.h>

#include <beer/beer_mem.h>
#include <beer/beer_iob.h>

int
beer_iob_init(struct beer_iob *iob, size_t size,
	     beer_iob_tx_t tx,
	     beer_iob_txv_t txv, void *ptr)
{
	iob->tx = tx;
	iob->txv = txv;
	iob->ptr = ptr;
	iob->size = size;
	iob->off = 0;
	iob->top = 0;
	iob->buf = NULL;
	if (size > 0) {
		iob->buf = beer_mem_alloc(size);
		if (iob->buf == NULL)
			return -1;
		memset(iob->buf, 0, size);
	}
	return 0;
}

void
beer_iob_clear(struct beer_iob *iob)
{
	iob->top = 0;
	iob->off = 0;
}

void
beer_iob_free(struct beer_iob *iob)
{
	if (iob->buf)
		beer_mem_free(iob->buf);
}
