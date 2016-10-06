#ifndef BEER_OBJECT_H_INCLUDED
#define BEER_OBJECT_H_INCLUDED

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
 * \file beer_object.h
 * \brief Object for manipulating msgpack objects
 */

#include <stdarg.h>

/**
 * \brief for internal use
 */
struct beer_sbo_stack {
	size_t   offset;
	uint32_t size;
	int8_t   type;
};

/**
 * \brief type of packing msgpack array/map
 *
 * - BEER_SBO_SIMPLE - without packing, demanding size to be specified
 * - BEER_SBO_SPARSE - 5 bytes always allocated for map/array, size is ignored
 * - BEER_SBO_PACKED - 1 byte is alloced for map/array, if needed more, then
 *                    everything is moved to n bytes, when called
 *                    "beer_object_container_close"
 */
enum beer_sbo_type {
	BEER_SBO_SIMPLE = 0,
	BEER_SBO_SPARSE,
	BEER_SBO_PACKED,
};

struct beer_sbuf_object {
	struct beer_sbo_stack *stack;
	uint8_t stack_size;
	uint8_t stack_alloc;
	enum beer_sbo_type type;
};

#define BEER_OBJ_CAST(SB) ((struct beer_sbuf_object *)(SB)->subdata)
#define BEER_SOBJ_CAST(S) BEER_OBJ_CAST(BEER_SBUF_CAST(S))

/**
 * \brief Set type of packing for objects
 *
 * Type must be set before first value was written
 *
 * \param s    beer_object instance
 * \param type type of packing
 *
 * \returns status of operation
 * \retval  -1 (something was written before
 * \retval   0 success
 */
int
beer_object_type(struct beer_stream *s, enum beer_sbo_type type);

/**
 * \brief create and initialize beer_object
 *
 * beer_object is used to create msgpack values: keys/tuples/args for
 * passing them into beer_request or beer_<operation>
 * if stream object is NULL, then new stream object will be created
 *
 * \param s object pointer
 *
 * \returns object pointer
 * \retval NULL error
 */
struct beer_stream *
beer_object(struct beer_stream *s);

/**
 * \brief Add nil to a stream object
 */
ssize_t
beer_object_add_nil(struct beer_stream *s);

/**
 * \brief Add integer to a stream object
 */
ssize_t
beer_object_add_int(struct beer_stream *s, int64_t value);

/**
 * \brief Add unsigned integer to a stream object
 */
ssize_t
beer_object_add_uint(struct beer_stream *s, uint64_t value);

/**
 * \brief Add string to a stream object
 */
ssize_t
beer_object_add_str(struct beer_stream *s, const char *str, uint32_t len);

/**
 * \brief Add null terminated string to a stream object
 */
ssize_t
beer_object_add_strz(struct beer_stream *s, const char *strz);

/**
 * \brief Add binary object to a stream object
 */
ssize_t
beer_object_add_bin(struct beer_stream *s, const void *bin, uint32_t len);

/**
 * \brief Add boolean to a stream object
 */
ssize_t
beer_object_add_bool(struct beer_stream *s, char value);

/**
 * \brief Add floating value to a stream object
 */
ssize_t
beer_object_add_float(struct beer_stream *s, float value);

/**
 * \brief Add double precision floating value to a stream object
 */
ssize_t
beer_object_add_double(struct beer_stream *s, double value);

/**
 * \brief Append array header to stream object
 * \sa beer_sbo_type
 */
ssize_t
beer_object_add_array(struct beer_stream *s, uint32_t size);

/**
 * \brief Append map header to stream object
 * \sa beer_sbo_type
 */
ssize_t
beer_object_add_map(struct beer_stream *s, uint32_t size);

/**
 * \brief Close array/map in case BEER_SBO_PACKED/BEER_SBO_SPARSE were used
 * \sa beer_sbo_type
 */
ssize_t
beer_object_container_close(struct beer_stream *s);

/**
 * \brief create immutable beer_object from given buffer
 */
struct beer_stream *
beer_object_as(struct beer_stream *s, char *buf, size_t buf_len);

/**
 * \brief verify that object is valid msgpack structure
 * \param s object pointer
 * \param type -1 on check without validating type, otherwise `enum mp_type`
 */
int
beer_object_verify(struct beer_stream *s, int8_t type);

/**
 * \brief reset beer_object to basic state
 * this function doesn't deallocate memory, but instead it simply sets all
 * pointers to beginning
 */
int
beer_object_reset(struct beer_stream *s);

/**
 * \brief create beer_object from format string/values (va_list variation)
 *
 * \code{.c}
 * \*to get a msgpack array of two items: number 42 and map (0->"false, 2->"true")*\
 * beer_object_format(s, "[%d {%d%s%d%s}]", 42, 0, "false", 1, "true");
 * \endcode
 *
 * \param s   beer_object instance
 * \param fmt zero-end string, containing structure of resulting
 * msgpack and types of next arguments.
 * Format can contain '[' and ']' pairs, defining arrays,
 * '{' and '}' pairs, defining maps, and format specifiers, described below:
 * %d, %i - int
 * %u - unsigned int
 * %ld, %li - long
 * %lu - unsigned long
 * %lld, %lli - long long
 * %llu - unsigned long long
 * %hd, %hi - short
 * %hu - unsigned short
 * %hhd, %hhi - char (as number)
 * %hhu - unsigned char (as number)
 * %f - float
 * %lf - double
 * %b - bool
 * %s - zero-end string
 * %.*s - string with specified length
 * %% is ignored
 * %'smth else' assert and undefined behaviour
 * NIL - a nil value
 * all other symbols are ignored.
 *
 * \sa beer_object_vformat
 * \sa beer_object_format
 */
ssize_t
beer_object_format(struct beer_stream *s, const char *fmt, ...);

/**
 * \brief create beer_object from format string/values
 * \sa beer_object_vformat
 * \sa beer_object_format
 */
ssize_t
beer_object_vformat(struct beer_stream *s, const char *fmt, va_list vl);

#endif /* BEER_OBJECT_H_INCLUDED */
