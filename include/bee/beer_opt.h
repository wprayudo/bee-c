#ifndef BEER_OPT_H_INCLUDED
#define BEER_OPT_H_INCLUDED

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

#include <sys/time.h>

/**
 * \file beer_opt.h
 * \brief Networking layer options
 */

struct beer_iob;

/**
 * \brief Callback type for read (instead of reading from socket)
 *
 * \param b   context for read operation
 * \param buf buf to read to
 * \param len size to read
 *
 * \returns size that was read
 * \retval  -1 error, errno must be set
 */
typedef ssize_t (*recv_cb_t)(struct beer_iob *b, void *buf, size_t len);

/**
 * \brief Callback type for write (instead of writing into socket)
 *
 * \param b   context for write operation
 * \param buf buf to write
 * \param len size to write
 *
 * \returns size that was written
 * \retval  -1 error, errno must be set
 */
typedef ssize_t (*send_cb_t)(struct beer_iob *b, void *buf, size_t len);

/**
 * \brief Callback type for write with iovec (instead of writing into socket)
 *
 * \param b   context for write operation
 * \param buf iovec to write
 * \param len iovec len
 *
 * \returns size that was written
 * \retval  -1 error, errno must be set
 */
typedef ssize_t (*sendv_cb_t)(struct beer_iob *b, const struct iovec *iov, int iov_count);

/**
 * \brief Options for connection
 */
enum beer_opt_type {
	BEER_OPT_URI, /*!< Options for setting URI */
	BEER_OPT_TMOUT_CONNECT, /*!< Option for setting timeout on connect */
	BEER_OPT_TMOUT_RECV, /*!< Option for setting timeout on recv */
	BEER_OPT_TMOUT_SEND, /*!< Option for setting timeout in send */
	BEER_OPT_SEND_CB, /*!< callback, that's executed on send
			  * \sa send_cb_t
			  */
	BEER_OPT_SEND_CBV, /*!< callback, that's executed on send with iovector
			   * \sa sendv_cb_t
			   */
	BEER_OPT_SEND_CB_ARG, /*!< callback context for send */
	BEER_OPT_SEND_BUF, /*!< Option for setting send buffer size */
	BEER_OPT_RECV_CB,  /*!< callback, that's executed on recv */
	BEER_OPT_RECV_CB_ARG, /*!< callback context for recv
			      * \sa recv_cb_t
			      */
	BEER_OPT_RECV_BUF /*!< Option for setting recv buffer size */
};

/**
 * \internal
 * \brief structure, that is used for options
 */
struct beer_opt {
	const char *uristr;
	struct uri *uri;
	struct timeval tmout_connect;
	struct timeval tmout_recv;
	struct timeval tmout_send;
	void *send_cb;
	void *send_cbv;
	void *send_cb_arg;
	int send_buf;
	void *recv_cb;
	void *recv_cb_arg;
	int recv_buf;
};

/**
 * \internal
 */
int
beer_opt_init(struct beer_opt *opt);

/**
 * \internal
 */
void
beer_opt_free(struct beer_opt *opt);

/**
 * \internal
 */
int
beer_opt_set(struct beer_opt *opt, enum beer_opt_type name, va_list args);

#endif /* BEER_OPT_H_INCLUDED */
