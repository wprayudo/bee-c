#ifndef BEER_REQUEST_H_INCLUDED
#define BEER_REQUEST_H_INCLUDED

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
 * \file beer_request.h
 * \brief Request creation using connection schema
 */

#include <beer/beer_proto.h>

struct beer_request {
	struct {
		uint64_t sync; /*!< Request sync id. Generated when encoded */
		enum beer_request_t type; /*!< Request type */
	} hdr; /*!< fields for header */
	uint32_t space_id; /*!< Space number */
	uint32_t index_id; /*!< Index number */
	uint32_t offset; /*!< Offset for select */
	uint32_t limit; /*!< Limit for select */
	enum beer_iterator_t iterator; /*!< Iterator for select */
	/* Search key, proc name or eval expression */
	const char *key; /*!< Pointer for
			  * key for select/update/delete,
			  * procedure  for call,
			  * expression for eval,
			  * operations for upsert
			  */
	const char *key_end;
	struct beer_stream *key_object; /*!< Pointer for key object
					* if allocated inside requests
					* functions
					*/
	const char *tuple; /*!< Pointer for
			    * tuple for insert/replace,
			    * ops for update
			    * default tuple for upsert,
			    * args for eval/call
			    */
	const char *tuple_end;
	struct beer_stream *tuple_object; /*!< Pointer for tuple object
					  * if allocated inside requests
					  * functions
					  */
	int index_base; /*!< field offset for UPDATE */
	int alloc; /*!< allocation mark */
};

/**
 * \brief Allocate and initialize request object
 *
 * if request pointer is NULL, then new request will be created
 *
 * \param req    pointer to request
 * \param stream pointer to stream for schema (may be NULL)
 *
 * \returns pointer to request object
 * \retval  NULL memory allocation failure
 */
struct beer_request *
beer_request_init(struct beer_request *req);
/**
 * \brief Free request object
 *
 * \param req request object
 */
void
beer_request_free(struct beer_request *req);

/**
 * \brief Set request space from number
 *
 * \param req   request object
 * \param space space number
 *
 * \retval 0 ok
 * \sa beer_request_set_space
 */
int
beer_request_set_space(struct beer_request *req, uint32_t space);

/**
 * \brief Set request index from number
 *
 * \param req   request object
 * \param index index number
 *
 * \retval 0  ok
 * \sa beer_request_set_index
 */
int
beer_request_set_index(struct beer_request *req, uint32_t index);

/**
 * \brief Set offset for select
 *
 * \param req    request pointer
 * \param offset offset to set
 *
 * \retval 0 ok
 */
int
beer_request_set_offset(struct beer_request *req, uint32_t offset);

/**
 * \brief Set limit for select
 *
 * \param req   request pointer
 * \param limit limit to set
 *
 * \retval 0 ok
 */
int
beer_request_set_limit(struct beer_request *req, uint32_t limit);

/**
 * \brief Set iterator for select
 *
 * \param req  request pointer
 * \param iter iter to set
 *
 * \retval 0 ok
 */
int
beer_request_set_iterator(struct beer_request *req, enum beer_iterator_t iter);

/**
 * \brief Set index base for update/upsert operation
 *
 * \param req        request pointer
 * \param index_base field offset to set
 *
 * \retval 0 ok
 */
int
beer_request_set_index_base(struct beer_request *req, uint32_t index_base);

/**
 * \brief Set key from predefined object
 *
 * \param req request pointer
 * \param s   beer_object pointer
 *
 * \retval 0 ok
 */
int
beer_request_set_key(struct beer_request *req, struct beer_stream *s);

/**
 * \brief Set key from print-like function
 *
 * \param req request pointer
 * \param fmt format string
 * \param ... arguments for format string
 *
 * \retval 0  ok
 * \retval -1 oom/format error
 * \sa beer_object_format
 */
int
beer_request_set_key_format(struct beer_request *req, const char *fmt, ...);

/**
 * \brief Set function from string
 *
 * \param req  request pointer
 * \param func function string
 * \param flen function string length
 *
 * \retval 0 ok
 */
int
beer_request_set_func(struct beer_request *req, const char *func, uint32_t flen);

/**
 * \brief Set function from NULL-terminated string
 *
 * \param req  request pointer
 * \param func function string
 *
 * \retval 0 ok
 */
int
beer_request_set_funcz(struct beer_request *req, const char *func);

/**
 * \brief Set expression from string
 *
 * \param req  request pointer
 * \param expr expression string
 * \param elen expression string length
 *
 * \retval 0  ok
 * \retval -1 error
 */
int
beer_request_set_expr(struct beer_request *req, const char *expr, uint32_t elen);

/**
 * \brief Set expression from NULL-terminated string
 *
 * \param req  request pointer
 * \param expr expression string
 *
 * \retval 0  ok
 * \retval -1 error
 */
int
beer_request_set_exprz(struct beer_request *req, const char *expr);

/**
 * \brief Set tuple from predefined object
 *
 * \param req request pointer
 * \param s   beer_object pointer
 *
 * \retval 0 ok
 */
int
beer_request_set_tuple(struct beer_request *req, struct beer_stream *s);

/**
 * \brief Set tuple from print-like function
 *
 * \param req request pointer
 * \param fmt format string
 * \param ... arguments for format string
 *
 * \retval 0  ok
 * \retval -1 oom/format error
 * \sa beer_object_format
 */
int
beer_request_set_tuple_format(struct beer_request *req, const char *fmt, ...);

/**
 * \brief Set operations from predefined object
 *
 * \param req request pointer
 * \param s   beer_object pointer
 *
 * \retval 0 ok
 */
int
beer_request_set_ops(struct beer_request *req, struct beer_stream *s);

/**
 * \brief Encode request to stream object
 *
 * \param s   stream pointer
 * \param req request pointer
 *
 * \retval >0 ok, sync is returned
 * \retval -1 out of memory
 */
int64_t
beer_request_compile(struct beer_stream *s, struct beer_request *req);

/**
 * \brief Encode request to stream object.
 *
 * \param[in]  s    stream pointer
 * \param[in]  req  request pointer
 * \param[out] sync pointer to compiled request
 *
 * \retval 0  ok
 * \retval -1 out of memory
 */
int
beer_request_writeout(struct beer_stream *s, struct beer_request *req,
		     uint64_t *sync);
/**
 * \brief create select request object
 * \sa beer_request_init
 */
struct beer_request *
beer_request_select(struct beer_request *req);

/**
 * \brief create insert request object
 * \sa beer_request_init
 */
struct beer_request *
beer_request_insert(struct beer_request *req);

/**
 * \brief create replace request object
 * \sa beer_request_init
 */
struct beer_request *
beer_request_replace(struct beer_request *req);

/**
 * \brief create update request object
 * \sa beer_request_init
 */
struct beer_request *
beer_request_update(struct beer_request *req);

/**
 * \brief create delete request object
 * \sa beer_request_init
 */
struct beer_request *
beer_request_delete(struct beer_request *req);

/**
 * \brief create call request object
 * \sa beer_request_init
 */
struct beer_request *
beer_request_call(struct beer_request *req);

/**
 * \brief create call request object
 * \sa beer_request_init
 */
struct beer_request *
beer_request_call_16(struct beer_request *req);

/**
 * \brief create auth request object
 * \sa beer_request_init
 */
struct beer_request *
beer_request_auth(struct beer_request *req);

/**
 * \brief create eval request object
 * \sa beer_request_init
 */
struct beer_request *
beer_request_eval(struct beer_request *req);

/**
 * \brief create upsert request object
 * \sa beer_request_init
 */
struct beer_request *
beer_request_upsert(struct beer_request *req);

/**
 * \brief create ping request object
 * \sa beer_request_init
 */
struct beer_request *
beer_request_ping(struct beer_request *req);

#endif /* BEER_REQUEST_H_INCLUDED */
