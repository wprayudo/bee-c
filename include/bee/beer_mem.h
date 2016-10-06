#ifndef BEER_MEM_H_INCLUDED
#define BEER_MEM_H_INCLUDED

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

/**
 * \internal
 * \file beer_mem.h
 * \brief Basic memory functions
 */

#define beerfunction_unused __attribute__((unused))

#if !defined __GNUC_MINOR__ || defined __INTEL_COMPILER || \
	defined __SUNPRO_C || defined __SUNPRO_CC
#define BEER_GCC_VERSION(major, minor) 0
#else
#define BEER_GCC_VERSION(major, minor) (__GNUC__ > (major) || \
	(__GNUC__ == (major) && __GNUC_MINOR__ >= (minor)))
#endif

#if !defined(__has_builtin)
#define __has_builtin(x) 0 /* clang */
#endif

#if BEER_GCC_VERSION(2, 9) || __has_builtin(__builtin_expect)
#define beerlikely(x) __builtin_expect(!!(x), 1)
#define beerunlikely(x) __builtin_expect(!!(x), 0)
#else
#define beerlikely(x) (x)
#define beerunlikely(x) (x)
#endif

/**
 * \brief basic allocation function type
 *
 * \param ptr  pointer to allocation/deallocation block
 * \param size size of block to allocat/reallocate
 *
 * \retval pointer to newly alloced/realloced block
 * \retval NULL on error/free
 */
typedef void *(beer_allocator_t)(void *ptr, size_t size);

/**
 * \brief initialize memory allocation function
 */
void *
beer_mem_init(beer_allocator_t alloc);

/**
 * \brief Internal function
 */
void *
beer_mem_alloc(size_t size);

/**
 * \brief Internal function
 */
void *
beer_mem_realloc(void *ptr, size_t size);

/**
 * \brief Internal function
 */
char *
beer_mem_dup(char *sz);

/**
 * \brief Internal function
 */
void
beer_mem_free(void *ptr);

#endif /* BEER_MEM_H_INCLUDED */
