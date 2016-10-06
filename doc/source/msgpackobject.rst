-------------------------------------------------------------------------------
                             MessagePack layer
-------------------------------------------------------------------------------

The basic MessagePack object layer is implemented as a wrapper for the
``msgpuck`` library. This layer supports array/map traversal, dynamically
sized msgpack arrays, etc. For details, see `MessagePack specification`_ and
`msgpuck readme file`_.

=====================================================================
                      Introduction to MessagePack
=====================================================================

MessagePack is an efficient binary serialization format. It lets you exchange
data among multiple languages, like JSON does. But it's faster and smaller.
Small integers are encoded into a single byte, and typical short strings require
only one extra byte in addition to the strings themselves.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                         Types
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* ``integer`` (``int`` < 0 and ``uint`` >= 0) represents an integer
* ``nil`` represents ``nil``
* ``boolean`` represents ``true`` or ``false``
* ``float`` (4 bytes) and ``double`` (8 bytes) represent a floating-point number
* ``string`` represents a UTF-8 string
* ``binary`` represents a byte array
* ``array`` represents a sequence of objects
* ``map`` represents key-value pairs of objects
* ``extension`` represents a tuple of type information and a byte array where
  type information is an integer whose meaning is defined by applications

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                      Limitations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* a value of an Integer object is limited from ``-(2^63)`` up to ``(2^64)-1``
* a value of a Float object is an IEEE 754 single or double precision
  floating-point number
* maximum length of a Binary object is ``(2^32)-1``
* maximum byte size of a String object is ``(2^32)-1``
* a String object may contain invalid byte sequence, and the behavior of a
  deserializer depends on the actual implementation when it receives an invalid
  byte sequence
* deserializers should provide the functionality to get the original byte array,
  so that applications can decide how to handle the object
* maximum number of elements of an Array object is ``(2^32)-1``
* maximum number of key-value associations of a Map object is ``(2^32)-1``

=====================================================================
                          Object creation
=====================================================================

.. // See beer/beer_object.c

.. c:function:: struct beer_stream *beer_object(struct beer_stream *s)

    Create an empty MsgPack object. If ``s`` is passed as ``NULL``, then the
    object is allocated. Otherwise, the allocated object is initialized.

.. c:function:: struct beer_stream *beer_object_as(struct beer_stream *s, char *buf, size_t buf_len)

    Create a read-only MsgPack object from buffer.
    The buffer's length is set in bytes. The source string isn't copied, so be
    careful not to destroy it while this function is running.

=====================================================================
                        Scalar MessagePack types
=====================================================================

.. c:function:: ssize_t beer_object_add_nil(struct beer_stream *s)

    Append ``nil`` to a stream object.

.. c:function:: ssize_t beer_object_add_int(struct beer_stream *s, int64_t value)
                ssize_t beer_object_add_uint(struct beer_stream *s, uint64_t value)

    For ``int64_t`` version: Append an integer to a stream object. If
    ``value >= 0``, then pack it in ``uint`` MsgPack type, otherwise the type
    is ``int``.

    For ``uint64_t`` version: Append unsigned integer (that's ``uint`` MsgPack
    type)

.. c:function:: ssize_t beer_object_add_str (struct beer_stream *s, const char *str, uint32_t len)
                ssize_t beer_object_add_strz(struct beer_stream *s, const char *str)

    Append a UTF-8 string to a stream object. If you use the :func:`<...>_strz`
    function, the string's length is calculated using ``strlen(str)``.

.. c:function:: ssize_t beer_object_add_bin(struct beer_stream *s, const char *bin, uint32_t len)

    Append a byte array to a stream object. The array's length is set in bytes.

.. c:function:: ssize_t beer_object_add_bool(struct beer_stream *s, char value)

    Append a boolean value to a stream object. If ``value == 0``, then append
    ``false``, otherwise ``true``.

.. c:function:: ssize_t beer_object_add_float(struct beer_stream *s, float val)
                ssize_t beer_object_add_double(struct beer_stream *s, double val)

    Append a float/double value to a stream object.

    * ``float`` means a 4-byte floating point number.
    * ``double`` means a 8-byte floating point number.

=====================================================================
                        Array/Map manipulation
=====================================================================

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                    Array/Map in MessagePack
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To understand why there are many problems when working with MessagePack
arrays/maps with dynamic size, we need to understand how it's originally
specified.

Arrays/maps are a sequence of elements following the "header". The problem is
that the header's length varies depending on the number of elements in the
sequence.

For example:

* length(elements) < 16 => length(header) == 1 byte
* length(elements) < (2^16) => length(header) == 3 bytes
* length(elements) < (2^32) => length(header) == 5 bytes

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                Working with Array/Map
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

So when you, dynamically, add 1 element and the sequence's length becomes 16 -
the header grows from 1 to 2 bytes (the same applies to 2^32). There are 3
strategies to work with it (each strategy corresponds to one of the 3 container
types):

.. containertype:: BEER_SBO_SIMPLE

    Set the sequence's size (stored in header) before adding elements into it.
    It's the default option.

.. containertype:: BEER_SBO_SPARSE

    Every container's header has a length of 5 bytes. It's recommended if you
    have very big tuples.

.. containertype:: BEER_SBO_PACKED

    When you're finished working with the container - it will be packed.

.. c:function:: int beer_object_type(struct beer_stream *s, enum BEER_SBO_TYPE type)

    Function for setting an object type. You can set it only when the container
    is empty.

    Returns -1 if it's not empty.

.. c:function:: ssize_t beer_object_add_array(struct beer_stream *s, uint32_t size)

    Append an array header to a stream object.

    The header's size is in bytes. If :containertype:`BEER_SBO_SPARSE` or
    :containertype:`BEER_SBO_PACKED` is set as container type, then size is
    ignored.

.. c:function:: ssize_t beer_object_add_map(struct beer_stream *s, uint32_t size)

    Append a map header to a stream object.

    The header's size is in bytes. If :containertype:`BEER_SBO_SPARSE` or
    :containertype:`BEER_SBO_PACKED` is set as container type, then size is
    ignored.

.. c:function:: ssize_t beer_object_container_close(struct beer_stream *s)

    Close the latest opened container. It's used when you set :func:`beer_object_type`
    to :containertype:`BEER_SBO_SPARSE` or :containertype:`BEER_SBO_PACKED` value.

=====================================================================
                        Object manipulation
=====================================================================

.. c:function:: ssize_t beer_object_format(struct beer_stream *s, const char *fmt, ...)
                ssize_t beer_object_vformat(struct beer_stream *s, const char *fmt, va_list vl)

    Append formatted msgpack values to the stream object. The
    :func:`<...>_vformat` function uses ``va_list`` as the third argument.

    Use the following symbols for formatting:

    * '[' and ']' pairs, defining arrays,
    * '{' and '}' pairs, defining maps
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

    Any other symbols are ignored.

.. c:function:: int beer_object_verify(struct beer_stream *s, int8_t type)

    Verify that an object is a valid msgpack structure. If ``type == -1``, then
    don't verify the first type, otherwise check that the first type is
    ``type``.

.. c:function:: int beer_object_reset(struct beer_stream *s)

    Reset a stream object to the basic state.

..  // Examples are commented out for a while as we currently revise them.
..  =====================================================================
..                             Example
..  =====================================================================

  .. literalinclude:: example.c
      :language: c
      :lines: 333-345

.. _MessagePack specification: https://github.com/msgpack/msgpack/blob/master/spec.md

.. _msgpuck readme file: https://github.com/wprayudo/msgpuck/blob/master/README.md
