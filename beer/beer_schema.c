#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <inttypes.h>
#include <assert.h>
#include <stdint.h>

#include <msgpuck.h>

#include <beer/beer_reply.h>
#include <beer/beer_stream.h>
#include <beer/beer_buf.h>
#include <beer/beer_object.h>
#include <beer/beer_proto.h>
#include <beer/beer_schema.h>
#include <beer/beer_mem.h>
#include <beer/beer_select.h>

#include "beer_assoc.h"

static inline void
beer_schema_ival_free(struct beer_schema_ival *val) {
	if (val) beer_mem_free((void *)val->name);
	beer_mem_free(val);
}

static inline void
beer_schema_index_free(struct mh_assoc_t *schema) {
	mh_int_t pos = 0;
	mh_int_t index_slot = 0;
	mh_foreach(schema, pos) {
		struct beer_schema_ival *ival =
			(*mh_assoc_node(schema, pos))->data;
		struct assoc_val *av1 = NULL, *av2 = NULL;
		do {
			struct assoc_key key_number = {
				(void *)&(ival->number),
				sizeof(uint32_t)
			};
			index_slot = mh_assoc_find(schema, &key_number, NULL);
			if (index_slot == mh_end(schema))
				break;
			av1 = *mh_assoc_node(schema, index_slot);
			mh_assoc_del(schema, index_slot, NULL);
		} while (0);
		do {
			struct assoc_key key_string = {
				ival->name,
				ival->name_len
			};
			index_slot = mh_assoc_find(schema, &key_string, NULL);
			if (index_slot == mh_end(schema))
				break;
			av2 = *mh_assoc_node(schema, index_slot);
			mh_assoc_del(schema, index_slot, NULL);
		} while (0);
		beer_schema_ival_free(ival);
		if (av1) beer_mem_free((void *)av1);
		if (av2) beer_mem_free((void *)av2);
	}
}

static inline void
beer_schema_sval_free(struct beer_schema_sval *val) {
	if (val) {
		beer_mem_free(val->name);
		if (val->index) {
			beer_schema_index_free(val->index);
			mh_assoc_delete(val->index);
		}
	}
	beer_mem_free(val);
}

static inline void
beer_schema_space_free(struct mh_assoc_t *schema) {
	mh_int_t pos = 0;
	mh_int_t space_slot = 0;
	mh_foreach(schema, pos) {
		struct beer_schema_sval *sval = NULL;
		sval = (*mh_assoc_node(schema, pos))->data;
		struct assoc_val *av1 = NULL, *av2 = NULL;
		do {
			struct assoc_key key_number = {
				(void *)&(sval->number),
				sizeof(uint32_t)
			};
			space_slot = mh_assoc_find(schema, &key_number, NULL);
			if (space_slot == mh_end(schema))
				break;
			av1 = *mh_assoc_node(schema, space_slot);
			mh_assoc_del(schema, space_slot, NULL);
		} while (0);
		do {
			struct assoc_key key_string = {
				sval->name,
				sval->name_len
			};
			space_slot = mh_assoc_find(schema, &key_string, NULL);
			if (space_slot == mh_end(schema))
				break;
			av2 = *mh_assoc_node(schema, space_slot);
			mh_assoc_del(schema, space_slot, NULL);
		} while (0);
		beer_schema_sval_free(sval);
		if (av1) beer_mem_free((void *)av1);
		if (av2) beer_mem_free((void *)av2);
	}
}

static inline int
beer_schema_add_space(struct mh_assoc_t *schema, const char **data)
{
	struct beer_schema_sval *space = NULL;
	struct assoc_val *space_string = NULL, *space_number = NULL;
	const char *tuple = *data;
	if (mp_typeof(*tuple) != MP_ARRAY)
		goto error;
	uint32_t tuple_len = mp_decode_array(&tuple); (void )tuple_len;
	space = beer_mem_alloc(sizeof(struct beer_schema_sval));
	if (!space)
		goto error;
	memset(space, 0, sizeof(struct beer_schema_sval));
	if (mp_typeof(*tuple) != MP_UINT)
		goto error;
	space->number = mp_decode_uint(&tuple);
	mp_next(&tuple); /* skip owner id */
	if (mp_typeof(*tuple) != MP_STR)
		goto error;
	const char *name_tmp = mp_decode_str(&tuple, &space->name_len);
	space->name = beer_mem_alloc(space->name_len);
	if (!space->name)
		goto error;
	memcpy(space->name, name_tmp, space->name_len);

	space->index = mh_assoc_new();
	if (!space->index)
		goto error;
	space_string = beer_mem_alloc(sizeof(struct assoc_val));
	if (!space_string)
		goto error;
	space_string->key.id     = space->name;
	space_string->key.id_len = space->name_len;
	space_string->data = space;
	space_number = beer_mem_alloc(sizeof(struct assoc_val));
	if (!space_number)
		goto error;
	space_number->key.id = (void *)&(space->number);
	space_number->key.id_len = sizeof(space->number);
	space_number->data = space;
	mh_assoc_put(schema, (const struct assoc_val **)&space_string,
		     NULL, NULL);
	mh_assoc_put(schema, (const struct assoc_val **)&space_number,
		     NULL, NULL);
	mp_next(data);
	return 0;
error:
	mp_next(data);
	beer_schema_sval_free(space);
	if (space_string) beer_mem_free(space_string);
	if (space_number) beer_mem_free(space_number);
	return -1;
}

int beer_schema_add_spaces(struct beer_schema *schema_obj, struct beer_reply *r) {
	struct mh_assoc_t *schema = schema_obj->space_hash;
	const char *tuple = r->data;
	if (mp_check(&tuple, tuple + (r->data_end - r->data)))
		return -1;
	tuple = r->data;
	if (mp_typeof(*tuple) != MP_ARRAY)
		return -1;
	uint32_t space_count = mp_decode_array(&tuple);
	while (space_count-- > 0) {
		if (beer_schema_add_space(schema, &tuple))
			return -1;
	}
	return 0;
}

static inline int
beer_schema_add_index(struct mh_assoc_t *schema, const char **data) {
	const struct beer_schema_sval *space = NULL;
	struct beer_schema_ival *index = NULL;
	struct assoc_val *index_number = NULL, *index_string = NULL;
	const char *tuple = *data;
	if (mp_typeof(*tuple) != MP_ARRAY)
		goto error;
	int64_t tuple_len = mp_decode_array(&tuple); (void )tuple_len;
	uint32_t space_number = mp_decode_uint(&tuple);
	if (mp_typeof(*tuple) != MP_UINT)
		goto error;
	struct assoc_key space_key = {
		(void *)&(space_number),
		sizeof(uint32_t)
	};
	mh_int_t space_slot = mh_assoc_find(schema, &space_key, NULL);
	if (space_slot == mh_end(schema))
		return -1;
	space = (*mh_assoc_node(schema, space_slot))->data;
	index = beer_mem_alloc(sizeof(struct beer_schema_ival));
	if (!index)
		goto error;
	memset(index, 0, sizeof(struct beer_schema_ival));
	if (mp_typeof(*tuple) != MP_UINT)
		goto error;
	index->number = mp_decode_uint(&tuple);
	if (mp_typeof(*tuple) != MP_STR)
		goto error;
	const char *name_tmp = mp_decode_str(&tuple, &index->name_len);
	index->name = beer_mem_alloc(index->name_len);
	if (!index->name)
		goto error;
	memcpy((void *)index->name, name_tmp, index->name_len);

	index_string = beer_mem_alloc(sizeof(struct assoc_val));
	if (!index_string) goto error;
	index_string->key.id     = index->name;
	index_string->key.id_len = index->name_len;
	index_string->data = index;
	index_number = beer_mem_alloc(sizeof(struct assoc_val));
	if (!index_number) goto error;
	index_number->key.id     = (void *)&(index->number);
	index_number->key.id_len = sizeof(uint32_t);
	index_number->data = index;
	mh_assoc_put(space->index, (const struct assoc_val **)&index_string,
		     NULL, NULL);
	mh_assoc_put(space->index, (const struct assoc_val **)&index_number,
		     NULL, NULL);
	mp_next(data);
	return 0;
error:
	mp_next(data);
	if (index_string) beer_mem_free(index_string);
	if (index_number) beer_mem_free(index_number);
	beer_schema_ival_free(index);
	return -1;
}

int beer_schema_add_indexes(struct beer_schema *schema_obj, struct beer_reply *r) {
	struct mh_assoc_t *schema = schema_obj->space_hash;
	const char *tuple = r->data;
	if (mp_check(&tuple, tuple + (r->data_end - r->data)))
		return -1;
	tuple = r->data;
	if (mp_typeof(*tuple) != MP_ARRAY)
		return -1;
	uint32_t space_count = mp_decode_array(&tuple);
	while (space_count-- > 0) {
		if (beer_schema_add_index(schema, &tuple))
			return -1;
	}
	return 0;
}

int32_t beer_schema_stosid(struct beer_schema *schema_obj, const char *name,
			  uint32_t name_len) {
	struct mh_assoc_t *schema = schema_obj->space_hash;
	struct assoc_key space_key = {name, name_len};
	mh_int_t space_slot = mh_assoc_find(schema, &space_key, NULL);
	if (space_slot == mh_end(schema))
		return -1;
	const struct beer_schema_sval *space =
		(*mh_assoc_node(schema, space_slot))->data;
	return space->number;
}

int32_t beer_schema_stoiid(struct beer_schema *schema_obj, uint32_t sid,
			  const char *name, uint32_t name_len) {
	struct mh_assoc_t *schema = schema_obj->space_hash;
	struct assoc_key space_key = {(void *)&sid, sizeof(uint32_t)};
	mh_int_t space_slot = mh_assoc_find(schema, &space_key, NULL);
	if (space_slot == mh_end(schema))
		return -1;
	const struct beer_schema_sval *space =
		(*mh_assoc_node(schema, space_slot))->data;
	struct assoc_key index_key = {name, name_len};
	mh_int_t index_slot = mh_assoc_find(space->index, &index_key, NULL);
	if (index_slot == mh_end(space->index))
		return -1;
	const struct beer_schema_ival *index =
		(*mh_assoc_node(space->index, index_slot))->data;
	return index->number;
}

struct beer_schema *beer_schema_new(struct beer_schema *s) {
	int alloc = (s == NULL);
	if (!s) {
		s = beer_mem_alloc(sizeof(struct beer_schema));
		if (!s) return NULL;
	}
	s->space_hash = mh_assoc_new();
	s->alloc = alloc;
	return s;
}

void beer_schema_flush(struct beer_schema *obj) {
	beer_schema_space_free(obj->space_hash);
}

void beer_schema_free(struct beer_schema *obj) {
	beer_schema_space_free(obj->space_hash);
	mh_assoc_delete(obj->space_hash);
}

ssize_t
beer_get_space(struct beer_stream *s)
{
	struct beer_stream *obj = beer_object(NULL);
	if (obj == NULL)
		return -1;

	beer_object_add_array(obj, 0);
	ssize_t retval = beer_select(s, beer_vsp_space, beer_vin_name,
				    UINT32_MAX, 0, BEER_ITER_ALL, obj);
	beer_stream_free(obj);
	return retval;
}

ssize_t
beer_get_index(struct beer_stream *s)
{
	struct beer_stream *obj = beer_object(NULL);
	if (obj == NULL)
		return -1;

	beer_object_add_array(obj, 0);
	ssize_t retval = beer_select(s, beer_vsp_index, beer_vin_name,
				    UINT32_MAX, 0, BEER_ITER_ALL, obj);
	beer_stream_free(obj);
	return retval;
}
