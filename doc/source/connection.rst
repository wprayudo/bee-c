-------------------------------------------------------------------------------
                         Networking layer
-------------------------------------------------------------------------------

The basic networking layer with batching support is implemented in multiple
parts:

* :file:`beer_net.c` - client-side layer
* :file:`beer_io.c`  - network abstraction layer
* :file:`beer_iob.c` - network buffer layer
* :file:`beer_opt.c` - network options layer

=====================================================================
                        Creating a connection
=====================================================================

.. c:function:: struct beer_stream *beer_net(struct beer_stream *s)

    Create a bee connection object. If ``s`` is NULL, then allocate memory
    for it.

    Return NULL if can't allocate memory.

=====================================================================
                          Handling errors
=====================================================================

.. see beer/beer_net.c

Possible error codes:

.. errtype:: BEER_EOK

    Not an error.

.. errtype:: BEER_EFAIL

    Failed to parse URI, bad protocol for sockets, or bad configuration option
    for the :func:`beer_set` function.

.. errtype:: BEER_EMEMORY

    Memory allocation failed.

.. errtype:: BEER_ESYSTEM

    System error, ``_errno`` will be set. Acquire it with the :func:`beer_errno`
    function.

.. errtype:: BEER_EBIG

    Read/write fragment is too big (in case the send/read buffer is smaller than
    the fragment you're trying to write into/read from).

.. errtype:: BEER_ESIZE

    Buffer size is incorrect.

.. errtype:: BEER_ERESOLVE

    Failed to resolve the hostname (the function :func:`gethostbyname(2)`
    failed).

.. errtype:: BEER_ETMOUT

    Timeout was reached during connect/read/write operations.

.. errtype:: BEER_EBADVAL

    Currently unused.

.. errtype:: BEER_ELOGIN

    Authentication error.

.. errtype:: BEER_LAST

    Pointer to the final element of an enumerated data structure (enum).

Functions to work with errors:

.. c:function:: enum beer_error beer_error(struct beer_stream *s)

    Return the error code of the last stream operation.

.. c:function:: char *beer_strerror(struct beer_stream *s)

    Format the error as a string message. In case the error code is
    :errtype:`BEER_ESYSTEM`, then the driver uses the system function
    :func:`strerror()` to format the message.

.. c:function:: int beer_errno(struct beer_stream *s)

    Return the ``errno_`` of the last stream operation (in case the error code
    is :errtype:`BEER_ESYSTEM`).

=====================================================================
                Manipulating a connection
=====================================================================

.. see beer/beer_net.c

.. c:function:: int beer_set(struct beer_stream *s, int opt, ...)

    You can set the following options for a connection:

    * BEER_OPT_URI (``const char *``) - URI for connecting to
      :program:`bee`.
    * BEER_OPT_TMOUT_CONNECT (``struct timeval *``) - timeout on connecting.
    * BEER_OPT_TMOUT_SEND (``struct timeval *``) - timeout on sending.
    * BEER_OPT_SEND_CB (``ssize_t (*send_cb_t)(struct beer_iob *b, void *buf,
      size_t len)``) - a function to be called instead of writing into a socket;
      uses the buffer ``buf`` which is ``len`` bytes long.
    * BEER_OPT_SEND_CBV (``ssize_t (*sendv_cb_t)(struct beer_iob *b,
      const struct iovec *iov, int iov_count)``) - a function to be called
      instead of writing into a socket;
      uses multiple (``iov_count``) buffers passed in ``iov``.
    * BEER_OPT_SEND_BUF (``int``) - the maximum size (in bytes) of the buffer for
      outgoing messages.
    * BEER_OPT_SEND_CB_ARG (``void *``) - context for "send" callbacks.
    * BEER_OPT_RECV_CB (``ssize_t (*recv_cb_t)(struct beer_iob *b, void *buf,
      size_t len)``) - a function to be called instead of reading from a socket;
      uses the buffer ``buf`` which is ``len`` bytes long.
    * BEER_OPT_RECV_BUF (``int``) - the maximum size (in bytes) of the buffer for
      incoming messages.
    * BEER_OPT_RECV_CB_ARG (``void *``) - context for "receive" callbacks.

    Return -1 and store the error in the stream.
    The error code can be either :errtype:`BEER_EFAIL` if can't parse the URI or
    ``opt`` is not defined, or :errtype:`BEER_EMEMORY` if failed to allocate
    memory for the URI.

.. c:function:: int beer_connect(struct beer_stream *s)

    Connect to :program:`bee` with preconfigured and allocated settings.

    Return -1 in the following cases:

    * Can't connect
    * Can't read greeting
    * Can't authenticate (if login/password was provided with the URI)
    * OOM while authenticating and getting schema
    * Can't parse schema

.. c:function:: void beer_close(struct beer_stream *s)

    Close connection to :program:`bee`.

.. c:function:: ssize_t beer_flush(struct beer_stream *s)

    Flush all buffered data to the socket.

    Return -1 in case of network error.

.. c:function:: int beer_fd(struct beer_stream *s)

    Return the file descriptor of the connection.

.. c:function:: int beer_reload_schema(struct beer_stream *s)

    Reload the schema from server. Delete the old schema and download/parse
    a new schema from server.

    See also ":ref:`working_with_a_schema`".

.. c:function:: int32_t beer_get_spaceno(struct beer_stream *s, const char *space, size_t space_len)
                int32_t beer_get_indexno(struct beer_stream *s, int space, const char *index, size_t index_len)

    Get space/index number from their names.
    For :func:`beer_get_indexno`, specify the length of the space name (in bytes)
    in ``space_len``.
    For :func:`beer_get_indexno`, specify the space ID number in ``space`` and
    the length of the index name (in bytes) in ``index_len``.

=====================================================================
                        Freeing a connection
=====================================================================

Use the :func:`beer_stream_free` function to free a connection object.

..  // Examples are commented out for a while as we currently revise them.
..  =====================================================================
..                             Example
..  =====================================================================

  .. literalinclude:: example.c
      :language: c
      :lines: 61-76,347-351

  .. literalinclude:: example.c
      :language: c
      :lines: 16-25,34-42
