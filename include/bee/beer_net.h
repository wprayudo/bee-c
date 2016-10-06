#ifndef BEER_NET_H_INCLUDED
#define BEER_NET_H_INCLUDED

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
 * \file beer_net.h
 * \brief Basic bee client library header for network stream layer
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/time.h>

#include <beer/beer_opt.h>
#include <beer/beer_iob.h>

/**
 * \brief Internal error codes
 */
enum beer_error {
	BEER_EOK, /*!< Everything is OK */
	BEER_EFAIL, /*!< Fail */
	BEER_EMEMORY, /*!< Memory allocation failed */
	BEER_ESYSTEM, /*!< System error */
	BEER_EBIG, /*!< Buffer is too big */
	BEER_ESIZE, /*!< Bad buffer size */
	BEER_ERESOLVE, /*!< gethostbyname(2) failed */
	BEER_ETMOUT, /*!< Operation timeout */
	BEER_EBADVAL, /*!< Bad argument (value) */
	BEER_ELOGIN, /*!< Failed to login */
	BEER_LAST /*!< Not an error */
};

/**
 * \brief Network stream structure
 */
struct beer_stream_net {
	struct beer_opt opt; /*!< Options for connection */
	int connected; /*!< Connection status. 1 - true, 0 - false */
	int fd; /*!< fd of connection */
	struct beer_iob sbuf; /*!< Send buffer */
	struct beer_iob rbuf; /*!< Recv buffer */
	enum beer_error error; /*!< If retval == -1, then error is set. */
	int errno_; /*!< If BEER_ESYSTEM then errno_ is set */
	char *greeting; /*!< Pointer to greeting, if connected */
	struct beer_schema *schema; /*!< Collation for space/index string<->number */
	int inited; /*!< 1 if iob/schema were allocated */
};

/*!
 * \internal
 * \brief Cast beer_stream to beer_net
 */
#define BEER_SNET_CAST(S) ((struct beer_stream_net*)(S)->data)

/**
 * \brief Create beer_net stream instance
 *
 * \param s stream pointer, maybe NULL
 *
 * If stream pointer is NULL, then new stream will be created.
 *
 * \returns stream pointer
 * \retval NULL oom
 *
 * \code{.c}
 * struct beer_stream *beer = beer_net(NULL);
 * assert(beer);
 * assert(beer_set(s, BEER_OPT_URI, "login:passw@localhost:3302") != -1);
 * assert(beer_connect(s) != -1);
 * ...
 * beer_close(s);
 * \endcode
 */
struct beer_stream *
beer_net(struct beer_stream *s);

/**
 * \brief Set options for connection
 *
 * \param s   stream pointer
 * \param opt option to set
 * \param ... option value
 *
 * \returns status
 * \retval -1 error
 * \retval  0 ok
 * \sa enum beer_opt_type
 *
 * \code{.c}
 * assert(beer_set(s, BEER_OPT_SEND_BUF, 16*1024) != -1);
 * assert(beer_set(s, BEER_OPT_RECV_BUF, 16*1024) != -1);
 * assert(beer_set(s, BEER_OPT_URI, "login:passw@localhost:3302") != -1);
 * \endcode
 *
 * \note
 * URI format:
 * * "[login:password@]host:port" for tcp sockets
 * * "[login:password@]/tmp/socket_path.sock" for unix sockets
 * \sa enum beer_opt_type
 */
int
beer_set(struct beer_stream *s, int opt, ...);

/*!
 * \internal
 * \brief Initialize network stream
 *
 * It must happened before connection, but after options are set.
 * 1) creation of beer_iob's (sbuf,rbuf)
 * 2) schema creation
 *
 * \param s stream for initialization
 *
 * \returns status
 * \retval 0  ok
 * \retval -1 error (oom/einval)
 */
int
beer_init(struct beer_stream *s);

/**
 * \brief Connect to bee with preconfigured and allocated settings
 *
 * \param s stream pointer
 *
 * \retval 0  ok
 * \retval -1 error (network/oom)
 */
int
beer_connect(struct beer_stream *s);

/**
 * \brief Close connection
 * \param s stream pointer
 */
void
beer_close(struct beer_stream *s);

/**
 * \brief Send written to buffer queries
 *
 * \param s beer_stream
 *
 * \returns number of bytes written to socket
 * \retval -1 on network error
 */
ssize_t
beer_flush(struct beer_stream *s);

/**
 * \brief Get beer_net stream fd
 */
int
beer_fd(struct beer_stream *s);

/**
 * \brief Error accessor for beer_net stream
 */
enum beer_error
beer_error(struct beer_stream *s);

/**
 * \brief Format error as string
 */
char *
beer_strerror(struct beer_stream *s);

/**
 * \brief Get last errno on socket
 */
int
beer_errno(struct beer_stream *s);

/**
 * \brief Flush space/index schema and get it from server
 *
 * \param s stream pointer
 *
 * \returns result
 * \retval  -1 error
 * \retval  0  ok
 */
int
beer_reload_schema(struct beer_stream *s);

/**
 * \brief Get space number from space name
 *
 * \returns space number
 * \retval  -1 error
 */
int beer_get_spaceno(struct beer_stream *s, const char *space, size_t space_len);

/**
 * \brief Get index number from index name and spaceid
 *
 * \returns index number
 * \retval  -1 error
 */
int beer_get_indexno(struct beer_stream *s, int spaceno, const char *index,
		    size_t index_len);

#ifdef __cplusplus
}
#endif

#endif /* BEER_NET_H_INCLUDED */
