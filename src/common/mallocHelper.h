#pragma once

#include <stdlib.h>
#include <assert.h>

#if FF_HAVE_MALLOC_USABLE_SIZE || FF_HAVE_MSVC_MSIZE
    #if __has_include(<malloc.h>)
        #include <malloc.h>
    #else
        #include <malloc_np.h> // For DragonFly BSD
    #endif
#elif FF_HAVE_MALLOC_SIZE
    #include <malloc/malloc.h>
#endif

[[gnu::always_inline, gnu::nonnull(1)]]
static inline void ffWrapFree(const void* pPtr) {
    assert(pPtr);
    if (*(void**) pPtr) {
        free(*(void**) pPtr);
    }
}

#define FF_AUTO_FREE [[gnu::cleanup(ffWrapFree)]]

// ptr MUST be a malloc'ed pointer
static inline size_t ffMallocUsableSize(const void* ptr) {
    assert(ptr);
#if FF_HAVE_MALLOC_USABLE_SIZE
    return malloc_usable_size((void*) ptr);
#elif FF_HAVE_MALLOC_SIZE
    return malloc_size((void*) ptr);
#elif FF_HAVE_MSVC_MSIZE
    return _msize((void*) ptr);
#else
    (void) ptr;
    return 0; // Not supported
#endif
}

#if __cplusplus

    #include <new>
    #include <cstdlib>

    #if defined(_WIN32)
        #include <malloc.h>
    #endif

[[gnu::always_inline]]
static inline void* ffAlignedAlloc(size_t size, size_t alignment) noexcept {
    #if defined(_WIN32)
    return _aligned_malloc(size, alignment);
    #else
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) == 0) [[likely]] {
        return ptr;
    }
    return nullptr;
    #endif
}

[[gnu::always_inline]]
static inline void ffAlignedFree(void* ptr) noexcept {
    #if defined(_WIN32)
    _aligned_free(ptr);
    #else
    ::free(ptr);
    #endif
}

[[gnu::always_inline]]
void* operator new(size_t size) {
    if (void* ptr = ::malloc(size)) [[likely]] {
        return ptr;
    }
    std::abort();
}

[[gnu::always_inline]]
void operator delete(void* ptr) noexcept {
    ::free(ptr);
}

[[gnu::always_inline]]
void operator delete(void* ptr, size_t size) noexcept {
    (void) size;
    ::free(ptr);
}

[[gnu::always_inline]]
void* operator new(size_t size, const std::nothrow_t&) noexcept {
    return ::malloc(size);
}

[[gnu::always_inline]]
void operator delete(void* ptr, const std::nothrow_t&) noexcept {
    ::free(ptr);
}

[[gnu::always_inline]]
void operator delete(void* ptr, size_t size, const std::nothrow_t&) noexcept {
    (void) size;
    ::free(ptr);
}

[[gnu::always_inline]]
void* operator new[](size_t size) {
    if (void* ptr = ::malloc(size)) [[likely]] {
        return ptr;
    }
    std::abort();
}

[[gnu::always_inline]]
void operator delete[](void* ptr) noexcept {
    ::free(ptr);
}

[[gnu::always_inline]]
void operator delete[](void* ptr, size_t size) noexcept {
    (void) size;
    ::free(ptr);
}

[[gnu::always_inline]]
void* operator new[](size_t size, const std::nothrow_t&) noexcept {
    return ::malloc(size);
}

[[gnu::always_inline]]
void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
    ::free(ptr);
}

[[gnu::always_inline]]
void operator delete[](void* ptr, size_t size, const std::nothrow_t&) noexcept {
    (void) size;
    ::free(ptr);
}

    #if __cplusplus >= 201703L // std::align_val_t (over-aligned new/delete, C++17)

[[gnu::always_inline]]
void* operator new(size_t size, std::align_val_t alignment) {
    if (void* ptr = ffAlignedAlloc(size, alignment)) {
        return ptr;
    }
    std::abort();
}

[[gnu::always_inline]]
void operator delete(void* ptr, std::align_val_t) noexcept {
    ffAlignedFree(ptr);
}

[[gnu::always_inline]]
void operator delete(void* ptr, size_t size, std::align_val_t) noexcept {
    (void) size;
    ffAlignedFree(ptr);
}

[[gnu::always_inline]]
void* operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    return ffAlignedAlloc(size, static_cast<size_t>(alignment));
}

[[gnu::always_inline]]
void operator delete(void* ptr, std::align_val_t, const std::nothrow_t&) noexcept {
    ffAlignedFree(ptr);
}

[[gnu::always_inline]]
void operator delete(void* ptr, size_t size, std::align_val_t, const std::nothrow_t&) noexcept {
    (void) size;
    ffAlignedFree(ptr);
}

[[gnu::always_inline]]
void* operator new[](size_t size, std::align_val_t alignment) {
    if (void* ptr = ffAlignedAlloc(size, static_cast<size_t>(alignment))) {
        return ptr;
    }
    std::abort();
}

[[gnu::always_inline]]
void operator delete[](void* ptr, std::align_val_t) noexcept {
    ffAlignedFree(ptr);
}

[[gnu::always_inline]]
void operator delete[](void* ptr, size_t size, std::align_val_t) noexcept {
    (void) size;
    ffAlignedFree(ptr);
}

[[gnu::always_inline]]
void* operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    return ffAlignedAlloc(size, static_cast<size_t>(alignment));
}

[[gnu::always_inline]]
void operator delete[](void* ptr, std::align_val_t, const std::nothrow_t&) noexcept {
    ffAlignedFree(ptr);
}

[[gnu::always_inline]]
void operator delete[](void* ptr, size_t size, std::align_val_t, const std::nothrow_t&) noexcept {
    (void) size;
    ffAlignedFree(ptr);
}

    #endif // __cplusplus >= 201703L

#endif
