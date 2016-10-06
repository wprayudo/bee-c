-------------------------------------------------------------------------------
                    Using a buffer for requests
-------------------------------------------------------------------------------

You may need a stream buffer (``beer_buf``) if you don't need the networking
functionality of ``beer_net``, but you need to write requests or parse replies.

=====================================================================
                        Creating a buffer
=====================================================================

.. c:function:: struct beer_stream *beer_buf(struct beer_stream *s)

    Create a stream buffer.

.. c:function:: struct beer_stream *beer_buf_as(struct beer_stream *s, char *buf, size_t buf_len)

    Create an immutable stream buffer from the buffer ``buf``. It can be used
    for parsing responses.

=====================================================================
                        Writing requests
=====================================================================

Use the basic functions for building requests:

* :func:`beer_select` / :func:`beer_insert` (see ":ref:`working_with_beer_stream`")
* :c:type:`struct beer_request` / :func:`beer_request_compile` (see
  ":ref:`working_with_beer_request`")

=====================================================================
                        Parsing replies
=====================================================================

Use an iterator to iterate through replies in your stream buffer,
or use the stream's :func:`read_reply` method
(``stream->read_reply(struct beer_stream *stream, struct beer_reply *reply)``).
