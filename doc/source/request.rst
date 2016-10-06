.. _working_with_beer_stream:

-------------------------------------------------------------------------------
                        Building a request with "beer_stream"
-------------------------------------------------------------------------------

Functions in this section add commands by appending them to a ``beer_stream``
buffer. To send the buffered query to the server, you can use the
:func:`beer_flush` function and then call :func:`beer->read_reply`, for example.

=====================================================================
                      Adding requests (except UPDATE)
=====================================================================

.. c:function:: ssize_t beer_auth_raw(struct beer_stream *s, const char *user, int ulen, const char *pass, int plen, const char *base64_salt)
                ssize_t beer_auth(struct beer_stream *s, const char *user, int ulen, const char *pass, int plen)
                ssize_t beer_deauth(struct beer_stream *s)

    Two functions for adding an authentication request.
    ``ulen`` and ``plen`` are the lengths (in bytes) of user's name ``user`` and
    password ``pass``.

    ``beer_deauth`` is a shortcut for ``beer_auth(s, NULL, 0, NULL, 0)``.

.. c:function:: ssize_t beer_call(struct beer_stream *s, const char *proc, size_t plen, struct beer_stream *args)
                ssize_t beer_call_16(struct beer_stream *s, const char *proc, size_t plen, struct beer_stream *args)
                ssize_t beer_eval(struct beer_stream *s, const char *expr, size_t elen, struct beer_stream *args)

    Add a call/eval request.

.. c:function:: ssize_t beer_delete(struct beer_stream *s, uint32_t space, uint32_t index, struct beer_stream *key)

    Add a delete request.

.. c:function:: ssize_t beer_insert(struct beer_stream *s, uint32_t space, struct beer_stream *tuple)
                ssize_t beer_replace(struct beer_stream *s, uint32_t space, struct beer_stream *tuple)

    Add an insert/replace request.

.. c:function:: ssize_t beer_ping(struct beer_stream *s)

    Add a ping request.

.. c:function:: ssize_t beer_select(struct beer_stream *s, uint32_t space, uint32_t index, uint32_t limit, uint32_t offset, uint8_t iterator, struct beer_stream *key)

    Add a select request.

    ``limit`` is the number of requests to gather. For gathering all results,
    set ``limit`` = ``UINT32_MAX``.

    ``offset`` is the number of requests to skip.

    ``iterator`` is the :ref:`iterator type <beer_iterator_types>` to use.

=====================================================================
                       Adding an UPDATE request
=====================================================================

.. c:function:: ssize_t beer_update(struct beer_stream *s, uint32_t space, uint32_t index, struct beer_stream *key, struct beer_stream *ops)

    Basic function for adding an update request.

    ``ops`` must be a container gained with the :func:`beer_update_container`
    function.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                   Container for operations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. c:function:: struct beer_stream *beer_update_container(struct beer_stream *ops)

    Create an update container.

.. c:function:: int beer_update_container_close(struct beer_stream *ops)

    Finish working with the container.

.. c:function:: int beer_update_container_reset(struct beer_stream *ops)

    Reset the container's state.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                          Operations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. c:function:: ssize_t beer_update_bit(struct beer_stream *ops, uint32_t fieldno, char op, uint64_t value)

    Function for adding a byte operation.

    Possible ``op`` values are:

    * ``'&'`` - for binary AND
    * ``'|'`` - for binary OR
    * ``'^'`` - for binary XOR

.. c:function:: ssize_t beer_update_arith_int(struct beer_stream *ops, uint32_t fieldno, char op, int64_t value)
                ssize_t beer_update_arith_float(struct beer_stream *ops, uint32_t fieldno, char op, float value)
                ssize_t beer_update_arith_double(struct beer_stream *ops, uint32_t fieldno, char op, double value)

    Three functions for adding an arithmetic operation for a specific data type
    (integer, float or double).

    Possible ``op``'s are:

    * ``+`` - for addition
    * ``-`` - for subtraction

.. c:function:: ssize_t beer_update_delete(struct beer_stream *ops, uint32_t fieldno, uint32_t fieldcount)

    Add a delete operation for the update request.
    ``fieldcount`` is the number of fields to delete.

.. c:function:: ssize_t beer_update_insert(struct beer_stream *ops, uint32_t fieldno, struct beer_stream *val)

    Add an insert operation for the update request.

.. c:function:: ssize_t beer_update_assign(struct beer_stream *ops, uint32_t fieldno, struct beer_stream *val)

    Add an assign operation for the update request.

.. c:function:: ssize_t beer_update_splice(struct beer_stream *ops, uint32_t fieldno, uint32_t position, uint32_t offset, const char *buffer, size_t buffer_len)

    Add a splice operation for the update request.

    "Splice" means to remove ``offset`` bytes from position ``position`` in
    field ``fieldno`` and paste ``buffer`` in the room of this fragment.

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
