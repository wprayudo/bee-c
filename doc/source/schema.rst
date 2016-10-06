.. _working_with_a_schema:

-------------------------------------------------------------------------------
                            Working with a schema
-------------------------------------------------------------------------------

Schema is needed for mapping ``"space_name" -> "space_id"`` and
``("space_id", "index_name") -> "index_id"``.

=====================================================================
                        Creating a schema
=====================================================================

.. c:function:: struct beer_schema *beer_schema_new(struct beer_schema *sch)

    Allocate and initialize a schema object.

=====================================================================
                Creating requests for acquiring a schema
=====================================================================

.. c:function:: ssize_t beer_get_space(struct beer_stream *s)
                ssize_t beer_get_index(struct beer_stream *s)

    Construct a query for selecting values from a schema.
    These are shortcuts for:

    * :func:`beer_select(s, 281, 0, UINT32_MAX, 0, BEER_ITER_ALL, "\x90")`
    * :func:`beer_select(s, 289, 0, UINT32_MAX, 0, BEER_ITER_ALL, "\x90")`

    where ``281`` and ``289`` are the IDs of the spaces listing all spaces
    (``281``) and all indexes (``289``) in the current Bee instance.

=====================================================================
                        Adding responses
=====================================================================

.. c:function:: struct beer_schema_add_spaces(struct beer_schema *sch, struct beer_reply *r)
                struct beer_schema_add_indexes(struct beer_schema *sch, struct beer_reply *r)

    Add spaces or indices to a schema.

