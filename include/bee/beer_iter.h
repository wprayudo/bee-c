#ifndef BEER_ITER_H_INCLUDED
#define BEER_ITER_H_INCLUDED

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
 * \file beer_iter.h
 * \brief Custom iterator types (msgpack/reply)
 */

/*!
 * iterator types
 */
enum beer_iter_type {
	BEER_ITER_ARRAY,
	BEER_ITER_MAP,
	BEER_ITER_REPLY,
//	BEER_ITER_REQUEST,
//	BEER_ITER_STORAGE
};

/*!
 * \brief msgpack array iterator
 */
struct beer_iter_array {
	const char *data; /*!< pointer to the beginning of array */
	const char *first_elem; /*!< pointer to the first element of array */
	const char *elem; /*!< pointer to current element of array */
	const char *elem_end; /*!< pointer to current element end of array */
	uint32_t elem_count; /*!< number of elements in array */
	int cur_index; /*!< index of current element */
};

/* msgpack array iterator accessors */

/**
 * \brief access msgpack array iterator
 */
#define BEER_IARRAY(I) (&(I)->data.array)

/**
 * \brief access current element form iterator
 */
#define BEER_IARRAY_ELEM(I) BEER_IARRAY(I)->elem

/**
 * \brief access end of current element from iterator
 */
#define BEER_IARRAY_ELEM_END(I) BEER_IARRAY(I)->elem_end

/*!
 * \brief msgpack map iterator
 */
struct beer_iter_map {
	const char *data; /*!< pointer to the beginning of map */
	const char *first_key; /*!< pointer to the first key of map */
	const char *key; /*!< pointer to current key of map */
	const char *key_end; /*!< pointer to current key end */
	const char *value; /*!< pointer to current value of map */
	const char *value_end; /*!< pointer to current value end */
	uint32_t pair_count; /*!< number of key-values pairs in array */
	int cur_index; /*!< index of current pair */
};

/* msgpack array iterator accessors */

/**
 * \brief access msgpack map iterator
 */
#define BEER_IMAP(I) (&(I)->data.map)

/**
 * \brief access current key from iterator
 */
#define BEER_IMAP_KEY(I) BEER_IMAP(I)->key

/**
 * \brief access current key end from iterator
 */
#define BEER_IMAP_KEY_END(I) BEER_IMAP(I)->key_end

/**
 * \brief access current value from iterator
 */
#define BEER_IMAP_VAL(I) BEER_IMAP(I)->value

/**
 * \brief access current value end from iterator
 */
#define BEER_IMAP_VAL_END(I) BEER_IMAP(I)->value_end

/*!
 * \brief reply iterator
 */
struct beer_iter_reply {
	struct beer_stream *s; /*!< stream pointer */
	struct beer_reply r;   /*!< current reply */
};

/* reply iterator accessors */

/**
 * \brief access reply iterator
 */
#define BEER_IREPLY(I) (&(I)->data.reply)

/**
 * \brief access current reply form iterator
 */
#define BEER_IREPLY_PTR(I) &BEER_IREPLY(I)->r

/* request iterator */
// struct beer_iter_request {
// 	struct beer_stream *s; /* stream pointer */
// 	struct beer_request r; /* current request */
// };

/* request iterator accessors */
// #define BEER_IREQUEST(I) (&(I)->data.request)
// #define BEER_IREQUEST_PTR(I) &BEER_IREQUEST(I)->r
// #define BEER_IREQUEST_STREAM(I) BEER_IREQUEST(I)->s

/* storage iterator */
// struct beer_iter_storage {
// 	struct beer_stream *s; /* stream pointer */
// 	struct beer_tuple t;   /* current fetched tuple */
// };

/* storage iterator accessors */
// #define BEER_ISTORAGE(I) (&(I)->data.storage)
// #define BEER_ISTORAGE_TUPLE(I) &BEER_ISTORAGE(I)->t
// #define BEER_ISTORAGE_STREAM(I) BEER_ISTORAGE(I)->s

/**
 * \brief iterator status
 */
enum beer_iter_status {
	BEER_ITER_OK, /*!< iterator is ok */
	BEER_ITER_FAIL /*!< error or end of iteration */
};

/**
 * \brief Common iterator object
 */
struct beer_iter {
	enum beer_iter_type type; /*!< iterator type
				  * \sa enum beer_iter_type
				  */
	enum beer_iter_status status; /*!< iterator status
				      * \sa enum beer_iter_status
				      */
	int alloc; /*!< allocation mark */
	/* interface callbacks */
	int  (*next)(struct beer_iter *iter); /*!< callback for next element */
	void (*rewind)(struct beer_iter *iter); /*!< callback for rewind */
	void (*free)(struct beer_iter *iter); /*!< callback for free of custom iter type */
	/* iterator data */
	union {
		struct beer_iter_array array; /*!< msgpack array iterator */
		struct beer_iter_map map; /*!< msgpack map iterator */
		struct beer_iter_reply reply; /*!< reply iterator */
//		struct beer_iter_request request;
//		struct beer_iter_storage storage;
	} data;
};

/**
 * \brief create msgpack array iterator from object
 *
 * if iterator pointer is NULL, then new iterator will be created.
 *
 * \param i pointer to allocated structure
 * \param s beer_object/beer_buf instance with array to traverse
 *
 * \returns iterator pointer
 * \retval  NULL on error.
 */
struct beer_iter *
beer_iter_array_object(struct beer_iter *i, struct beer_stream *s);

/**
 * \brief create msgpack array iterator from pointer
 *
 * if iterator pointer is NULL, then new iterator will be created.
 *
 * \param i pointer to allocated structure
 * \param data pointer to data with array
 * \param size size of data (may be more, it won't go outside)
 *
 * \returns iterator pointer
 * \retval  NULL on error.
 */
struct beer_iter *
beer_iter_array(struct beer_iter *i, const char *data, size_t size);

/**
 * \brief create msgpack map iterator from object
 *
 * if iterator pointer is NULL, then new iterator will be created.
 *
 * \param i pointer to allocated structure
 * \param s beer_object/beer_buf instance with map to traverse
 *
 * \returns iterator pointer
 * \retval  NULL error.
 */
struct beer_iter *
beer_iter_map_object(struct beer_iter *i, struct beer_stream *s);

/**
 * \brief create msgpack map iterator from pointer
 *
 * if iterator pointer is NULL, then new iterator will be created.
 *
 * \param i pointer to allocated structure
 * \param data pointer to data with map
 * \param size size of data (may be more, it won't go outside)
 *
 * \returns iterator pointer
 * \retval  NULL error.
 */
struct beer_iter *
beer_iter_map(struct beer_iter *i, const char *data, size_t size);

/**
 * \brief create and initialize tuple reply iterator;
 *
 * \param i pointer to allocated structure
 * \param s beer_net stream pointer
 *
 * if stream iterator pointer is NULL, then new stream
 * iterator will be created.
 *
 * \returns stream iterator pointer
 * \retval NULL error.
*/
struct beer_iter *
beer_iter_reply(struct beer_iter *i, struct beer_stream *s);

// struct beer_iter *beer_iter_request(struct beer_iter *i, struct beer_stream *s);
// struct beer_iter *beer_iter_storag(struct beer_iter *i, struct beer_stream *s);

/**
 * \brief free iterator.
 *
 * \param i iterator pointer
 */
void
beer_iter_free(struct beer_iter *i);

/**
 * \brief iterate to next element in tuple
 *
 * \param i iterator pointer
 *
 * depend on iterator tuple, sets to the
 * next msgpack field or next response in the stream.
 *
 * \retval 0 end of iteration
 * \retval 1 next step of iteration
 */
int
beer_next(struct beer_iter *i);

/**
 * \brief reset iterator pos to beginning
 *
 * \param i iterator pointer
 */
void
beer_rewind(struct beer_iter *i);


#endif /* BEER_ITER_H_INCLUDED */
