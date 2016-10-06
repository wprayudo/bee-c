#ifndef BEER_BUF_H_INCLUDED
#define BEER_BUF_H_INCLUDED

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
 * \file beer_buf.h
 * \brief basic buffer structure
 */

/*!
 * Type for resize function
 */
typedef char *(*beer_buf_resize_t)(struct beer_stream *, size_t);

/*!
 * Stream buffer substructure
 */
struct beer_stream_buf {
	char   *data;   /*!< buffer data */
	size_t  size;   /*!< buffer used */
	size_t  alloc;  /*!< current buffer size */
	size_t  rdoff;  /*!< read offset */
	char *(*resize)(struct beer_stream *, size_t); /*!< resize function */
	void  (*free)(struct beer_stream *); /*!< custom free function */
	void   *subdata; /*!< subclass */
	int     as;      /*!< constructed from user's string */
};

/* buffer stream accessors */

/*!
 * \brief cast beer_stream to beer_stream_buf structure
 */
#define BEER_SBUF_CAST(S) ((struct beer_stream_buf *)(S)->data)
/*!
 * \brief get data field from beer_stream_buf
 */
#define BEER_SBUF_DATA(S) BEER_SBUF_CAST(S)->data
/*!
 * \brief get size field from beer_stream_buf
 */
#define BEER_SBUF_SIZE(S) BEER_SBUF_CAST(S)->size

/**
 * \brief Allocate and init stream buffer object
 *
 * if stream pointer is NULL, then new stream will be created.
 *
 * \param   s pointer to allocated stream buffer
 *
 * \returns pointer to newly allocated sbuf object
 * \retval  NULL memory allocation failure
 */
struct beer_stream *
beer_buf(struct beer_stream *s);

struct beer_stream *
beer_buf_as(struct beer_stream *s, char *buf, size_t buf_len);

#endif /* BEER_BUF_H_INCLUDED */
