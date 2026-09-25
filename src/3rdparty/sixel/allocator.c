/*
 * Copyright (c) 2014-2018 Hayaki Saito
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

/* STDC_HEADERS */
#include <stdlib.h>

#if HAVE_ASSERT_H
# include <assert.h>
#endif  /* HAVE_ASSERT_H */
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif  /* HAVE_SYS_TYPES_H */
#if HAVE_ERRNO_H
# include <errno.h>
#endif  /* HAVE_ERRNO_H */
#if HAVE_MEMORY_H
# include <memory.h>
#endif  /* HAVE_MEMORY_H */

#include "allocator.h"
#include "malloc_stub.h"

/* create allocator object */
SIXELSTATUS
sixel_allocator_new(
    sixel_allocator_t   /* out */ **ppallocator,  /* allocator object to be created */
    sixel_malloc_t      /* in */  fn_malloc,      /* custom malloc() function */
    sixel_calloc_t      /* in */  fn_calloc,      /* custom calloc() function */
    sixel_realloc_t     /* in */  fn_realloc,     /* custom realloc() function */
    sixel_free_t        /* in */  fn_free)        /* custom free() function */
{
    SIXELSTATUS status = SIXEL_FALSE;

    if (ppallocator == NULL) {
        sixel_helper_set_additional_message(
            "sixel_allocator_new: given argument ppallocator is null.");
        status = SIXEL_BAD_ARGUMENT;
        goto end;
    }

    if (fn_malloc == NULL) {
        fn_malloc = malloc;
    }

    if (fn_calloc == NULL) {
        fn_calloc = calloc;
    }

    if (fn_realloc == NULL) {
        fn_realloc = realloc;
    }

    if (fn_free == NULL) {
        fn_free = free;
    }

    *ppallocator = fn_malloc(sizeof(sixel_allocator_t));
    if (*ppallocator == NULL) {
        sixel_helper_set_additional_message(
            "sixel_allocator_new: fn_malloc() failed.");
        status = SIXEL_BAD_ALLOCATION;
        goto end;
    }

    (*ppallocator)->ref         = 1;
    (*ppallocator)->fn_malloc   = fn_malloc;
    (*ppallocator)->fn_calloc   = fn_calloc;
    (*ppallocator)->fn_realloc  = fn_realloc;
    (*ppallocator)->fn_free     = fn_free;

    status = SIXEL_OK;

end:
    return status;
}


/* destruct allocator object */
static void
sixel_allocator_destroy(
    sixel_allocator_t /* in */ *allocator)  /* allocator object to
                                               be destroyed */
{
    /* precondition */
    assert(allocator);
    assert(allocator->fn_free);

    allocator->fn_free(allocator);
}


/* increase reference count of allocatort object (thread-unsafe) */
SIXELAPI void
sixel_allocator_ref(
    sixel_allocator_t /* in */ *allocator)  /* allocator object to be
                                               increment reference counter */
{
    /* precondition */
    assert(allocator);

    /* TODO: be thread safe */
    ++allocator->ref;
}


/* decrease reference count of output context object (thread-unsafe) */
SIXELAPI void
sixel_allocator_unref(
    sixel_allocator_t /* in */ *allocator)  /* allocator object to be unreference */
{
    /* TODO: be thread safe */
    if (allocator) {
        assert(allocator->ref > 0);
        --allocator->ref;
        if (allocator->ref == 0) {
            sixel_allocator_destroy(allocator);
        }
    }
}


/* call custom malloc() */
SIXELAPI void *
sixel_allocator_malloc(
    sixel_allocator_t   /* in */ *allocator,  /* allocator object */
    size_t              /* in */ n)           /* allocation size */
{
    /* precondition */
    assert(allocator);
    assert(allocator->fn_malloc);

    if (n == 0) {
        sixel_helper_set_additional_message(
            "sixel_allocator_malloc: called with n == 0");
        return NULL;
    }

    if (n > SIXEL_ALLOCATE_BYTES_MAX) {
        return NULL;
    }

    return allocator->fn_malloc(n);
}


/* call custom calloc() */
SIXELAPI void *
sixel_allocator_calloc(
    sixel_allocator_t   /* in */ *allocator,  /* allocator object */
    size_t              /* in */ nelm,        /* number of elements */
    size_t              /* in */ elsize)      /* size of element */
{
    size_t n;

    /* precondition */
    assert(allocator);
    assert(allocator->fn_calloc);

    n = nelm * elsize;

    if (n == 0) {
        sixel_helper_set_additional_message(
            "sixel_allocator_malloc: called with n == 0");
        return NULL;
    }

    if (n > SIXEL_ALLOCATE_BYTES_MAX) {
        return NULL;
    }

    return allocator->fn_calloc(nelm, elsize);
}


/* call custom realloc() */
SIXELAPI void *
sixel_allocator_realloc(
    sixel_allocator_t   /* in */ *allocator,  /* allocator object */
    void                /* in */ *p,          /* existing buffer to be re-allocated */
    size_t              /* in */ n)           /* re-allocation size */
{
    /* precondition */
    assert(allocator);
    assert(allocator->fn_realloc);

    if (n == 0) {
        sixel_helper_set_additional_message(
            "sixel_allocator_malloc: called with n == 0");
        return NULL;
    }

    if (n > SIXEL_ALLOCATE_BYTES_MAX) {
        return NULL;
    }

    return allocator->fn_realloc(p, n);
}


/* call custom free() */
SIXELAPI void
sixel_allocator_free(
    sixel_allocator_t   /* in */ *allocator,  /* allocator object */
    void                /* in */ *p)          /* existing buffer to be freed */
{
    /* precondition */
    assert(allocator);
    assert(allocator->fn_free);

    allocator->fn_free(p);
}


#if HAVE_TESTS
volatile int sixel_debug_malloc_counter;

void *
sixel_bad_malloc(size_t size)
{
    return sixel_debug_malloc_counter-- == 0 ? NULL: malloc(size);
}


void *
sixel_bad_calloc(size_t count, size_t size)
{
    (void) count;
    (void) size;

    return NULL;
}


void *
sixel_bad_realloc(void *ptr, size_t size)
{
    (void) ptr;
    (void) size;

    return NULL;
}
#endif  /* HAVE_TESTS */

#if 0
int
rpl_posix_memalign(void **memptr, size_t alignment, size_t size)
{
#if HAVE_POSIX_MEMALIGN
    return posix_memalign(memptr, alignment, size);
#elif HAVE_ALIGNED_ALLOC
    *memptr = aligned_alloc(alignment, size);
    return *memptr ? 0: ENOMEM;
#elif HAVE_MEMALIGN
    *memptr = memalign(alignment, size);
    return *memptr ? 0: ENOMEM;
#elif HAVE__ALIGNED_MALLOC
    return _aligned_malloc(size, alignment);
#else
# error
#endif /* _MSC_VER */
}
#endif


#if HAVE_TESTS
static int
test1(void)
{
    int nret = EXIT_FAILURE;
    SIXELSTATUS status;
    sixel_allocator_t *allocator = NULL;

    status = sixel_allocator_new(NULL, malloc, calloc, realloc, free);
    if (status != SIXEL_BAD_ARGUMENT) {
        goto error;
    }

    status = sixel_allocator_new(&allocator, NULL, calloc, realloc, free);
    if (SIXEL_FAILED(status)) {
        goto error;
    }

    status = sixel_allocator_new(&allocator, malloc, NULL, realloc, free);
    if (SIXEL_FAILED(status)) {
        goto error;
    }

    status = sixel_allocator_new(&allocator, malloc, calloc, NULL, free);
    if (SIXEL_FAILED(status)) {
        goto error;
    }

    status = sixel_allocator_new(&allocator, malloc, calloc, realloc, NULL);
    if (SIXEL_FAILED(status)) {
        goto error;
    }

    nret = EXIT_SUCCESS;

error:
    return nret;
}


static int
test2(void)
{
    int nret = EXIT_FAILURE;
    SIXELSTATUS status;
    sixel_allocator_t *allocator = NULL;

    sixel_debug_malloc_counter = 1;

    status = sixel_allocator_new(&allocator, sixel_bad_malloc, calloc, realloc, free);
    if (status == SIXEL_BAD_ALLOCATION) {
        goto error;
    }

    nret = EXIT_SUCCESS;

error:
    return nret;
}


SIXELAPI int
sixel_allocator_tests_main(void)
{
    int nret = EXIT_FAILURE;
    size_t i;
    typedef int (* testcase)(void);

    static testcase const testcases[] = {
        test1,
        test2
    };

    for (i = 0; i < sizeof(testcases) / sizeof(testcase); ++i) {
        nret = testcases[i]();
        if (nret != EXIT_SUCCESS) {
            goto error;
        }
    }

    nret = EXIT_SUCCESS;

error:
    return nret;
}
#endif  /* HAVE_TESTS */

/* emacs Local Variables:      */
/* emacs mode: c               */
/* emacs tab-width: 4          */
/* emacs indent-tabs-mode: nil */
/* emacs c-basic-offset: 4     */
/* emacs End:                  */
/* vim: set expandtab ts=4 sts=4 sw=4 : */
/* EOF */
