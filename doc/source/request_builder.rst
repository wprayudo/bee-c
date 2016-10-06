.. _working_with_beer_request:

-------------------------------------------------------------------------------
                    Building a request with "beer_request"
-------------------------------------------------------------------------------

=====================================================================
                      Basic types
=====================================================================

.. _beer_iterator_types:

.. c:type:: enum beer_iterator_t

    Custom iterator type. Possible values:

    * **BEER_ITER_EQ** - Equality iterator
    * **BEER_ITER_REQ** - Reverse equality iterator
    * **BEER_ITER_ALL** - Receive all elements
    * **BEER_ITER_LT** - "Less than" iterator
    * **BEER_ITER_LE** - "Less or equal" iterator
    * **BEER_ITER_GE** - "Greater or equal" iterator
    * **BEER_ITER_GT** - "Greater than" iterator
    * **BEER_ITER_BITS_ALL_SET** - All specified bits are set (bitset specific)
    * **BEER_ITER_BITS_ANY_SET** - Any specified bits are set (bitset specific)
    * **BEER_ITER_BITS_ALL_NOT_SET** - All specified bits are not set (bitset
      specific)
    * **BEER_ITER_OVERLAP** - Search for tuples with overlapping rectangles
      (R-tree specific)
    * **BEER_ITER_NEIGHBOR** - Search for the nearest neighbour (R-tree specific)

.. c:type:: enum beer_request_type

    Request type. Possible values:

    * **BEER_OP_SELECT**
    * **BEER_OP_INSERT**
    * **BEER_OP_REPLACE**
    * **BEER_OP_UPDATE**
    * **BEER_OP_DELETE**
    * **BEER_OP_CALL**
    * **BEER_OP_CALL_16**
    * **BEER_OP_AUTH**
    * **BEER_OP_EVAL**
    * **BEER_OP_PING**

.. c:type:: struct beer_request

    Base request object. It contains all parts of a request.

    .. code-block:: c

        struct beer_request {
            struct {
                uint64_t sync;
                enum beer_request_type type;
            } hdr;
            uint32_t space_id;
            uint32_t index_id;
            uint32_t offset;
            uint32_t limit;
            enum beer_iterator_t iterator;
            const char * key;
            const char * key_end;
            struct beer_stream * key_object;
            const char * tuple;
            const char * tuple_end;
            struct beer_stream * tuple_object;
            int index_base;
            int alloc;
        };

    See field descriptions further in this section.

=====================================================================
                        Creating a request
=====================================================================

.. c:function:: struct beer_request *beer_request_init(struct beer_request *req)

    Allocate and initialize a request.

.. c:function:: struct beer_request *beer_request_select(struct beer_request *req)
                struct beer_request *beer_request_insert(struct beer_request *req)
                struct beer_request *beer_request_replace(struct beer_request *req)
                struct beer_request *beer_request_update(struct beer_request *req)
                struct beer_request *beer_request_delete(struct beer_request *req)
                struct beer_request *beer_request_call(struct beer_request *req)
                struct beer_request *beer_request_call_16(struct beer_request *req)
                struct beer_request *beer_request_auth(struct beer_request *req)
                struct beer_request *beer_request_eval(struct beer_request *req)
                struct beer_request *beer_request_upsert(struct beer_request *req)
                struct beer_request *beer_request_ping(struct beer_request *req)

    Shortcuts for allocating and initializing requests of specific types.

=====================================================================
                      Request header
=====================================================================

.. c:member:: uint64_t beer_request.hdr.sync

    Sync ID number of a request. Generated automatically when the request is
    compiled.

.. c:member:: enum beer_request_type beer_request.hdr.type

    Type of a request.

=====================================================================
                   User-defined request fields
=====================================================================

.. c:member:: uint32_t beer_request.space_id
              uint32_t beer_request.index_id
              uint32_t beer_request.offset
              uint32_t beer_request.limit

    Space and index ID numbers, offset and limit for SELECT (specified in
    records).

=====================================================================
                Set/get request fields and functions
=====================================================================

.. c:function:: int beer_request_set_iterator(struct beer_request *req, enum beer_iterator_t iter)

    Set an iterator type for SELECT.

    Field that is set in ``beer_request``:

    .. code-block:: c

        enum beer_iterator_t iterator;

.. c:function:: int beer_request_set_key(struct beer_request *req, struct beer_stream *s)
                int beer_request_set_key_format(struct beer_request *req, const char *fmt, ...)

    Set a key (both key start and end) for SELECT/UPDATE/DELETE from a stream
    object.

    Or set a key using the print-like function :func:`beer_object_vformat`.
    Take ``fmt`` format string followed by arguments for the format string.
    Return ``-1`` if the :func:`beer_object_vformat` function fails.

    Fields that are set in ``beer_request``:

    .. code-block:: c

        const char * key;
        const char * key_end;
        struct beer_stream * key_object; // set by `beer_request_set_key_format`

.. c:function:: int beer_request_set_tuple(struct beer_request *req, struct beer_stream *obj)
                int beer_request_set_tuple_format(struct beer_request *req, const char *fmt, ...)

    Set a tuple (both tuple start and end) for UPDATE/EVAL/CALL from a stream.

    Or set a tuple using the print-like function :func:`beer_object_vformat`.
    Take ``fmt`` format string followed by arguments for the format string.
    Return ``-1`` if the :func:`beer_object_vformat` function fails.

    * For UPDATE, the tuple is a stream object with operations.
    * For EVAL/CALL, the tuple is a stream object with arguments.

    Fields that are set in ``beer_request``:

    .. code-block:: c

        const char * tuple;
        const char * tuple_end;
        struct beer_stream * tuple_object;  // set by `beer_request_set_tuple_format`

.. c:function:: int beer_request_set_expr (struct beer_request *req, const char *expr, size_t len)
                int beer_request_set_exprz(struct beer_request *req, const char *expr)

    Set an expression (both expression start and end) for EVAL from a string.

    If the function ``<...>_exprz`` is used, then length is calculated using
    :func:`strlen(str)`. Otherwise, ``len`` is the expression's length (in
    bytes).

    Return ``-1`` if ``expr`` is not :func:`beer_request_evaluate`.

    Fields that are set in ``beer_request``:

    .. code-block:: c

        const char * key;
        const char * key_end;
        struct beer_stream * key_object; // set by `beer_request_set_exprz`

.. c:function:: int beer_request_set_func (struct beer_request *req, const char *func, size_t len)
                int beer_request_set_funcz(struct beer_request *req, const char *func)

    Set a function (both function start and end) for CALL from a string.

    If the function ``<...>_funcz`` is used, then length is calculated using
    :func:`strlen(str)`. Otherwise, ``len`` is the function's length (in bytes).

    Return ``-1`` if ``func`` is not :func:`beer_request_call`.

    Fields that are set in ``beer_request``:

    .. code-block:: c

        const char * key;
        const char * key_end;
        struct beer_stream * key_object; // set by `beer_request_set_funcz`

.. c:function:: int beer_request_set_ops(struct beer_request *req, struct beer_stream *s)

    Set operations (both operations start and end) for UPDATE/UPSERT from a
    stream.

    Fields that are set in ``beer_request``:

    .. code-block:: c

        const char * key;
        const char * key_end;

.. c:function:: int beer_request_set_index_base(struct beer_request *req, uint32_t index_base)

    Set an index base (field offset) for UPDATE/UPSERT.

    Field that is set in ``beer_request``:

    .. code-block:: c

        int index_base;

=====================================================================
                       Manipulating a request
=====================================================================

.. c:function:: uint64_t beer_request_compile(struct beer_stream *s, struct beer_request *req)

    Compile a request into a stream.

    Return ``-1`` if bad command or can't write to stream.

.. c:function:: void beer_request_free(struct beer_request *req)

    Free a request object.

..  // Examples are commented out for a while as we currently revise them.
..  =====================================================================
..                             Example
..  =====================================================================

  Examples here are common for building requests with both ``beer_stream`` and
  ``beer_request`` objects.

  .. literalinclude:: example.c
      :language: c
      :lines: 157,171-174

  .. literalinclude:: example.c
      :language: c
      :lines: 187-202

  .. literalinclude:: example.c
      :language: c
      :lines: 225-226,230-250,255-259

  .. literalinclude:: example.c
      :language: c
      :lines: 279,281-293,298-306
