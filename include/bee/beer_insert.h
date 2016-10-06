#ifndef BEER_INSERT_H_INCLUDED
#define BEER_INSERT_H_INCLUDED

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
 * \file beer_insert.h
 * \brief Insert/Replace request
 */

/**
 * \brief Construct insert request and write it into stream
 *
 * \param s     stream object to write request to
 * \param space space no to insert tuple into
 * \param tuple (beer_object instance) msgpack array with tuple to insert to
 *
 * \retval number of bytes written to stream
 */
ssize_t
beer_insert(struct beer_stream *s, uint32_t space, struct beer_stream *tuple);

/**
 * \brief Construct replace request and write it into stream
 *
 * \param s     stream object to write request to
 * \param space space no to replace tuple into
 * \param tuple (beer_object instance) msgpack array with tuple to replace to
 *
 * \retval number of bytes written to stream
 */
ssize_t
beer_replace(struct beer_stream *s, uint32_t space, struct beer_stream *tuple);

#endif /* BEER_INSERT_H_INCLUDED */
