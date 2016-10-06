-------------------------------------------------------------------------------
                        Using the basic stream object
-------------------------------------------------------------------------------

=====================================================================
                            Basic type
=====================================================================

.. c:type:: struct beer_stream

    .. code-block:: c

        struct beer_stream {
            int alloc;
            ssize_t (* write)(struct beer_stream * s, const char * buf, size_t size);
            ssize_t (* writev)(struct beer_stream * s, struct iovec * iov, int count);
            ssize_t (* read)(struct beer_stream * s, char * buf, size_t size);
            int (* read_reply)(struct beer_stream * s, struct beer_reply * r);
            void (* free)(struct beer_stream * s);
            void * data;
            uint32_t wrcnt;
            uint64_t reqid;
        };

.. c:member:: int beer_stream.alloc

    Allocation mark, equals zero if memory isn't allocated for the stream
    object. Otherwise not zero.

.. c:member:: void * beer_stream.data

    Subclass data.

.. c:member:: uint_32t beer_stream.wrcnt

    Counter of write operations.

.. c:member:: uint_64t beer_stream.reqid

    Request ID number of a current operation.
    Incremented at every request compilation. Default is zero.

=====================================================================
                    Working with a stream
=====================================================================

.. c:function:: void beer_stream_free(struct beer_stream *s)

    Common free function for all stream objects.

.. c:function:: uint32_t beer_stream_reqid(struct beer_stream *s, uint32_t reqid)

    Increment and set the request ID that'll be used to construct a query,
    and return the previous request ID.

.. c:function:: ssize_t beer_stream.write(struct beer_stream * s, const char * buf, size_t size)
                ssize_t beer_stream.writev(struct beer_stream * s, struct iovec * iov, int count)

    Write a string or a vector to a stream.
    Here ``size`` is the string's size (in bytes), and ``count`` is the number
    of records in the ``iov`` vector.

.. c:function:: ssize_t beer_stream.read(struct beer_stream * s, char * buf, size_t size)
                int beer_stream.read_reply(struct beer_stream * s, struct beer_reply * r)

    Read a reply from server. The :func:`read` function simply writes the reply
    to a string buffer, while the :func:`read_reply` function parses it and
    writes to a ``beer_reply`` data structure.

..  // Examples are commented out for a while as we currently revise them.
..  =====================================================================
..                             Example
..  =====================================================================

  .. literalinclude:: example.c
      :language: c
      :lines: 350
