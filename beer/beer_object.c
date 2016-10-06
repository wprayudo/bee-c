#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <msgpuck.h>

#include <beer/beer_reply.h>
#include <beer/beer_stream.h>
#include <beer/beer_buf.h>
#include <beer/beer_object.h>
#include <beer/beer_mem.h>

static void
beer_sbuf_object_free(struct beer_stream *s)
{
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);
	if (sbo->stack) beer_mem_free(sbo->stack);
	sbo->stack = NULL;
	beer_mem_free(sbo);
}

int
beer_object_type(struct beer_stream *s, enum beer_sbo_type type)
{
	if (s->wrcnt > 0) return -1;
	BEER_SOBJ_CAST(s)->type = type;
	return 0;
};

static int
beer_sbuf_object_grow_stack(struct beer_sbuf_object *sbo)
{
	if (sbo->stack_alloc == 128) return -1;
	uint8_t new_stack_alloc = 2 * sbo->stack_alloc;
	struct beer_sbo_stack *stack = beer_mem_alloc(new_stack_alloc * sizeof(
				struct beer_sbo_stack));
	if (!stack) return -1;
	sbo->stack_alloc = new_stack_alloc;
	sbo->stack = stack;
	return 0;
}

static char *
beer_sbuf_object_resize(struct beer_stream *s, size_t size) {
	struct beer_stream_buf *sb = BEER_SBUF_CAST(s);
	if (sb->size + size > sb->alloc) {
		size_t newsize = 2 * (sb->alloc);
		if (newsize < sb->size + size)
			newsize = sb->size + size;
		char *nd = beer_mem_realloc(sb->data, newsize);
		if (nd == NULL) {
			beer_mem_free(sb->data);
			return NULL;
		}
		sb->data = nd;
		sb->alloc = newsize;
	}
	return sb->data + sb->size;
}

struct beer_stream *
beer_object(struct beer_stream *s)
{
	if ((s = beer_buf(s)) == NULL)
		goto error;

	struct beer_stream_buf *sb = BEER_SBUF_CAST(s);
	sb->resize = beer_sbuf_object_resize;
	sb->free = beer_sbuf_object_free;

	struct beer_sbuf_object *sbo = beer_mem_alloc(sizeof(struct beer_sbuf_object));
	if (sbo == NULL)
		goto error;
	sb->subdata = sbo;
	sbo->stack_size = 0;
	sbo->stack_alloc = 8;
	sbo->stack = beer_mem_alloc(sbo->stack_alloc *
			sizeof(struct beer_sbo_stack));
	if (sbo->stack == NULL)
		goto error;
	beer_object_type(s, BEER_SBO_SIMPLE);

	return s;
error:
	beer_stream_free(s);
	return NULL;
}

ssize_t
beer_object_add_nil (struct beer_stream *s)
{
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);
	if (sbo->stack_size > 0)
		sbo->stack[sbo->stack_size - 1].size += 1;
	char data[2]; char *end = mp_encode_nil(data);
	return s->write(s, data, end - data);
}

ssize_t
beer_object_add_uint(struct beer_stream *s, uint64_t value)
{
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);
	if (sbo->stack_size > 0)
		sbo->stack[sbo->stack_size - 1].size += 1;
	char data[10], *end;
	end = mp_encode_uint(data, value);
	return s->write(s, data, end - data);
}

ssize_t
beer_object_add_int (struct beer_stream *s, int64_t value)
{
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);
	if (sbo->stack_size > 0)
		sbo->stack[sbo->stack_size - 1].size += 1;
	char data[10], *end;
	if (value < 0)
		end = mp_encode_int(data, value);
	else
		end = mp_encode_uint(data, value);
	return s->write(s, data, end - data);
}

ssize_t
beer_object_add_str (struct beer_stream *s, const char *str, uint32_t len)
{
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);
	if (sbo->stack_size > 0)
		sbo->stack[sbo->stack_size - 1].size += 1;
	struct iovec v[2]; int v_sz = 2;
	char data[6], *end;
	end = mp_encode_strl(data, len);
	v[0].iov_base = data;
	v[0].iov_len  = end - data;
	v[1].iov_base = (void *)str;
	v[1].iov_len  = len;
	return s->writev(s, v, v_sz);
}

ssize_t
beer_object_add_strz (struct beer_stream *s, const char *strz)
{
	uint32_t len = strlen(strz);
	return beer_object_add_str(s, strz, len);
}

ssize_t
beer_object_add_bin (struct beer_stream *s, const void *bin, uint32_t len)
{
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);
	if (sbo->stack_size > 0)
		sbo->stack[sbo->stack_size - 1].size += 1;
	struct iovec v[2]; int v_sz = 2;
	char data[6], *end;
	end = mp_encode_binl(data, len);
	v[0].iov_base = data;
	v[0].iov_len  = end - data;
	v[1].iov_base = (void *)bin;
	v[1].iov_len  = len;
	return s->writev(s, v, v_sz);
}

ssize_t
beer_object_add_bool (struct beer_stream *s, char value)
{
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);
	if (sbo->stack_size > 0)
		sbo->stack[sbo->stack_size - 1].size += 1;
	char data[2], *end;
	end = mp_encode_bool(data, value != 0);
	return s->write(s, data, end - data);
}

ssize_t
beer_object_add_float (struct beer_stream *s, float value)
{
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);
	if (sbo->stack_size > 0)
		sbo->stack[sbo->stack_size - 1].size += 1;
	char data[6], *end;
	end = mp_encode_float(data, value);
	return s->write(s, data, end - data);
}

ssize_t
beer_object_add_double (struct beer_stream *s, double value)
{
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);
	if (sbo->stack_size > 0)
		sbo->stack[sbo->stack_size - 1].size += 1;
	char data[10], *end;
	end = mp_encode_double(data, value);
	return s->write(s, data, end - data);
}

static char *
mp_encode_array32(char *data, uint32_t size)
{
	data = mp_store_u8(data, 0xdd);
	return mp_store_u32(data, size);
}

ssize_t
beer_object_add_array (struct beer_stream *s, uint32_t size)
{
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);
	if (sbo->stack_size > 0)
		sbo->stack[sbo->stack_size - 1].size += 1;
	char data[6], *end;
	struct beer_stream_buf  *sb  = BEER_SBUF_CAST(s);
	if (sbo->stack_size == sbo->stack_alloc)
		if (beer_sbuf_object_grow_stack(sbo) == -1)
			return -1;
	sbo->stack[sbo->stack_size].size = 0;
	sbo->stack[sbo->stack_size].offset = sb->size;
	sbo->stack[sbo->stack_size].type = MP_ARRAY;
	sbo->stack_size += 1;
	if (BEER_SOBJ_CAST(s)->type == BEER_SBO_SIMPLE) {
		end = mp_encode_array(data, size);
	} else if (BEER_SOBJ_CAST(s)->type == BEER_SBO_SPARSE) {
		end = mp_encode_array32(data, 0);
	} else if (BEER_SOBJ_CAST(s)->type == BEER_SBO_PACKED) {
		end = mp_encode_array(data, 0);
	} else {
		return -1;
	}
	ssize_t rv = s->write(s, data, end - data);
	return rv;
}

static char *
mp_encode_map32(char *data, uint32_t size)
{
	data = mp_store_u8(data, 0xdf);
	return mp_store_u32(data, size);
}

ssize_t
beer_object_add_map (struct beer_stream *s, uint32_t size)
{
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);
	if (sbo->stack_size > 0)
		sbo->stack[sbo->stack_size - 1].size += 1;
	char data[6], *end;
	struct beer_stream_buf  *sb  = BEER_SBUF_CAST(s);
	if (sbo->stack_size == sbo->stack_alloc)
		if (beer_sbuf_object_grow_stack(sbo) == -1)
			return -1;
	sbo->stack[sbo->stack_size].size = 0;
	sbo->stack[sbo->stack_size].offset = sb->size;
	sbo->stack[sbo->stack_size].type = MP_MAP;
	sbo->stack_size += 1;
	if (BEER_SOBJ_CAST(s)->type == BEER_SBO_SIMPLE) {
		end = mp_encode_map(data, size);
	} else if (BEER_SOBJ_CAST(s)->type == BEER_SBO_SPARSE) {
		end = mp_encode_map32(data, 0);
	} else if (BEER_SOBJ_CAST(s)->type == BEER_SBO_PACKED) {
		end = mp_encode_map(data, 0);
	} else {
		return -1;
	}
	ssize_t rv = s->write(s, data, end - data);
	return rv;
}

ssize_t
beer_object_container_close (struct beer_stream *s)
{
	struct beer_stream_buf   *sb = BEER_SBUF_CAST(s);
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);
	if (sbo->stack_size == 0) return -1;
	size_t       size   = sbo->stack[sbo->stack_size - 1].size;
	enum mp_type type   = sbo->stack[sbo->stack_size - 1].type;
	size_t       offset = sbo->stack[sbo->stack_size - 1].offset;
	if (type == MP_MAP && size % 2) return -1;
	sbo->stack_size -= 1;
	char *lenp = sb->data + offset;
	if (sbo->type == BEER_SBO_SIMPLE) {
		return 0;
	} else if (sbo->type == BEER_SBO_SPARSE) {
		if (type == MP_MAP)
			mp_encode_map32(lenp, size/2);
		else
			mp_encode_array32(lenp, size);
		return 0;
	} else if (sbo->type == BEER_SBO_PACKED) {
		size_t sz = 0;
		if (type == MP_MAP)
			sz = mp_sizeof_map(size/2);
		else
			sz = mp_sizeof_array(size);
		if (sz > 1) {
			if (!sb->resize(s, sz - 1))
				return -1;
			lenp = sb->data + offset;
			memmove(lenp + sz, lenp + 1, sb->size - offset - 1);
		}
		if (type == MP_MAP) {
			mp_encode_map(sb->data + offset, size/2);
		} else {
			mp_encode_array(sb->data + offset, size);
		}
		sb->size += (sz - 1);
		return 0;
	}
	return -1;
}

int beer_object_verify(struct beer_stream *obj, int8_t type)
{
	const char *pos = BEER_SBUF_DATA(obj);
	const char *end = pos + BEER_SBUF_SIZE(obj);
	if (type >= 0 && mp_typeof(*pos) != (uint8_t) type) return -1;
	if (mp_check(&pos, end)) return -1;
	if (pos < end) return -1;
	return 0;
}

ssize_t beer_object_vformat(struct beer_stream *s, const char *fmt, va_list vl)
{
	if (beer_object_type(s, BEER_SBO_PACKED) == -1)
		return -1;
	ssize_t result = 0, rv = 0;

	for (const char *f = fmt; *f; f++) {
		if (f[0] == '[') {
			if ((rv = beer_object_add_array(s, 0)) == -1)
				return -1;
			result += rv;
		} else if (f[0] == '{') {
			if ((rv = beer_object_add_map(s, 0)) == -1)
				return -1;
			result += rv;
		} if (f[0] == ']' || f[0] == '}') {
			if ((rv = beer_object_container_close(s)) == -1)
				return -1;
			result += rv;
		} else if (f[0] == '%') {
			f++;
			assert(f[0]);
			int64_t int_value = 0;
			int int_status = 0; /* 1 - signed, 2 - unsigned */

			if (f[0] == 'd' || f[0] == 'i') {
				int_value = va_arg(vl, int);
				int_status = 1;
			} else if (f[0] == 'u') {
				int_value = va_arg(vl, unsigned int);
				int_status = 2;
			} else if (f[0] == 's') {
				const char *str = va_arg(vl, const char *);
				uint32_t len = (uint32_t)strlen(str);
				if ((rv = beer_object_add_str(s, str, len)) == -1)
					return -1;
				result += rv;
			} else if (f[0] == '.' && f[1] == '*' && f[2] == 's') {
				uint32_t len = va_arg(vl, uint32_t);
				const char *str = va_arg(vl, const char *);
				if ((rv = beer_object_add_str(s, str, len)) == -1)
					return -1;
				result += rv;
				f += 2;
			} else if (f[0] == 'f') {
				float v = (float)va_arg(vl, double);
				if ((rv = beer_object_add_float(s, v)) == -1)
					return -1;
				result += rv;
			} else if (f[0] == 'l' && f[1] == 'f') {
				double v = va_arg(vl, double);
				if ((rv = beer_object_add_double(s, v)) == -1)
					return -1;
				result += rv;
				f++;
			} else if (f[0] == 'b') {
				bool v = (bool)va_arg(vl, int);
				if ((rv = beer_object_add_bool(s, v)) == -1)
					return -1;
				result += rv;
			} else if (f[0] == 'l'
				   && (f[1] == 'd' || f[1] == 'i')) {
				int_value = va_arg(vl, long);
				int_status = 1;
				f++;
			} else if (f[0] == 'l' && f[1] == 'u') {
				int_value = va_arg(vl, unsigned long);
				int_status = 2;
				f++;
			} else if (f[0] == 'l' && f[1] == 'l'
				   && (f[2] == 'd' || f[2] == 'i')) {
				int_value = va_arg(vl, long long);
				int_status = 1;
				f += 2;
			} else if (f[0] == 'l' && f[1] == 'l' && f[2] == 'u') {
				int_value = va_arg(vl, unsigned long long);
				int_status = 2;
				f += 2;
			} else if (f[0] == 'h'
				   && (f[1] == 'd' || f[1] == 'i')) {
				int_value = va_arg(vl, int);
				int_status = 1;
				f++;
			} else if (f[0] == 'h' && f[1] == 'u') {
				int_value = va_arg(vl, unsigned int);
				int_status = 2;
				f++;
			} else if (f[0] == 'h' && f[1] == 'h'
				   && (f[2] == 'd' || f[2] == 'i')) {
				int_value = va_arg(vl, int);
				int_status = 1;
				f += 2;
			} else if (f[0] == 'h' && f[1] == 'h' && f[2] == 'u') {
				int_value = va_arg(vl, unsigned int);
				int_status = 2;
				f += 2;
			} else if (f[0] != '%') {
				/* unexpected format specifier */
				assert(false);
			}

			if (int_status) {
				if ((rv = beer_object_add_int(s, int_value)) == -1)
					return -1;
				result += rv;
			}
		} else if (f[0] == 'N' && f[1] == 'I' && f[2] == 'L') {
			if ((rv = beer_object_add_nil(s)) == -1)
				return -1;
			result += rv;
			f += 2;
		}
	}
	return result;
}

ssize_t beer_object_format(struct beer_stream *s, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ssize_t res = beer_object_vformat(s, fmt, args);
	va_end(args);
	return res;
}

struct beer_stream *beer_object_as(struct beer_stream *s, char *buf,
				 size_t buf_len)
{
	if (s == NULL) {
		s = beer_object(s);
		if (s == NULL)
			return NULL;
	}
	struct beer_stream_buf *sb = BEER_SBUF_CAST(s);

	sb->data = buf;
	sb->size = buf_len;
	sb->alloc = buf_len;
	sb->as = 1;

	return s;
}

int beer_object_reset(struct beer_stream *s)
{
	struct beer_stream_buf *sb = BEER_SBUF_CAST(s);
	struct beer_sbuf_object *sbo = BEER_SOBJ_CAST(s);

	s->reqid = 0;
	s->wrcnt = 0;
	sb->size = 0;
	sb->rdoff = 0;
	sbo->stack_size = 0;
	sbo->type = BEER_SBO_SIMPLE;

	return 0;
}
