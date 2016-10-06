
/*
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <beer/beer_mem.h>

static void *custom_realloc(void *ptr, size_t size) {
	if (!ptr) {
		if (!size)
			return NULL;
		return calloc(1, size);
	}
	if (size)
		return realloc(ptr, size);
	free(ptr);
	return NULL;
}

/*
static void *(*_beer_realloc)(void *ptr, size_t size) =
	(void *(*)(void*, size_t))realloc;
*/

static void *(*_beer_realloc)(void *ptr, size_t size) = custom_realloc;

void *beer_mem_init(beer_allocator_t alloc) {
	void *ptr = _beer_realloc;
	if (alloc)
		_beer_realloc = alloc;
	return ptr;
}

void *beer_mem_alloc(size_t size) {
	return _beer_realloc(NULL, size);
}

void *beer_mem_realloc(void *ptr, size_t size) {
	return _beer_realloc(ptr, size);
}

char *beer_mem_dup(char *sz) {
	size_t len = strlen(sz);
	char *szp = beer_mem_alloc(len + 1);
	if (szp == NULL)
		return NULL;
	memcpy(szp, sz, len + 1);
	return szp;
}

void beer_mem_free(void *ptr) {
	_beer_realloc(ptr, 0);
}
