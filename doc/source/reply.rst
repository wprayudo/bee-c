-------------------------------------------------------------------------------
                            Parsing a reply
-------------------------------------------------------------------------------

=====================================================================
                          Basic type
=====================================================================

.. c:type:: struct beer_reply

    Base reply object.

    .. code-block:: c

        struct beer_reply {
            const char * buf;
            size_t buf_size;
            uint64_t code;
            uint64_t sync;
            uint64_t schema_id;
            const char * error;
            const char * error_end;
            const char * data;
            const char * data_end;
        };

.. c:member:: const char *beer_reply.buf

    Pointer to a buffer with reply data.
    It's needed for the function :func:`beer_reply`.

.. c:member:: size_t beer_reply.buf_size

    Size of the buffer with reply data, in bytes.
    It's needed for the function :func:`beer_reply`.

.. c:member:: uint64_t beer_reply.code

    The return code of a query.

    If ``code == 0`` then it's ok, otherwise use the macro
    :c:macro:`BEER_REPLY_ERROR` to convert it to an error code.

    If ``code > 0``, then ``error`` and ``error_end`` must be set.
    Otherwise ``data`` and ``data_end`` is set.

.. c:member:: uint64_t beer_reply.sync

    Sync of a query. Generated automatically when the query is sent, and so it
    comes back with the reply.

.. c:member:: uint64_t beer_reply.schema_id

    Schema ID of a query. This is the number of the
    :ref:`schema <working_with_a_schema>` revision.

.. c:member:: const char *beer_reply.error
              const char *beer_reply.error_end

    Pointers to an error string in case of ``code != 0``.
    See all error codes in the file
    `/src/box/errcode.h <https://github.com/wprayudo/bee/blob/1.6/src/box/errcode.h>`_)
    in the main Bee project.

.. c:member:: const char *beer_reply.data
              const char *beer_reply.data_end

    Query data. This is a MessagePack object. Parse it with any msgpack library,
    e.g. ``msgpuck``.

=====================================================================
                     Manipulating a reply
=====================================================================

.. c:function:: struct beer_reply *beer_reply_init(struct beer_reply *r)

    Initialize a reply request.

.. c:function:: void beer_reply_free(struct beer_reply *r)

    Free a reply request.

.. c:function:: int beer_reply(struct beer_reply *r, char *buf, size_t size, size_t *off)

    Parse ``size`` bytes of an iproto reply from the buffer ``buf`` (it must
    contain a full reply).
    In ``off``, return the number of bytes remaining in the reply (if processed
    all ``size`` bytes), or the number of processed bytes (if processing
    failed).

.. c:function:: int beer_reply_from(struct beer_reply *r, beer_reply_t rcv, void *ptr)

    Parse an iproto reply from the ``rcv`` callback and with the context
    ``ptr``.

.. c:macro:: BEER_REPLY_ERR(reply)

    Return an error code (number, shifted right) converted from
    ``beer_reply.code``.

..  // Examples are commented out for a while as we currently revise them.
..  =====================================================================
..                             Example
..  =====================================================================

  .. literalinclude:: example.c
      :language: c
      :lines: 159,164-167,179-183

  .. literalinclude:: example.c
      :language: c
      :lines: 209-220
