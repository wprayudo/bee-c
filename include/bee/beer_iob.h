#ifndef BEER_IOB_H_INCLUDED
#define BEER_IOB_H_INCLUDED

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

/**
 * \internal
 * \file beer_iob.h
 * \brief Basic network layer static sized buffer
 */

typedef ssize_t (*beer_iob_tx_t)(void *ptr, const char *buf, size_t size);
typedef ssize_t (*beer_iob_txv_t)(void *ptr, struct iovec *iov, int count);

struct beer_iob {
	char *buf;
	size_t off;
	size_t top;
	size_t size;
	beer_iob_tx_t tx;
	beer_iob_txv_t txv;
	void *ptr;
};

int
beer_iob_init(struct beer_iob *iob, size_t size, beer_iob_tx_t tx,
	     beer_iob_txv_t txv, void *ptr);

void
beer_iob_clear(struct beer_iob *iob);

void
beer_iob_free(struct beer_iob *iob);

#endif /* BEER_IOB_H_INCLUDED */
