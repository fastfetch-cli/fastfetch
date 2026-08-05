#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define FF_LIST_DEFAULT_ALLOC 16

typedef struct FFlist {
    uint8_t* data;
    uint32_t length;
    uint32_t capacity;
} FFlist;

// Removes the first element, and copy its value to `*result`
bool ffListShift(FFlist* list, uint32_t elementSize, void* __restrict result);
// Removes the last element, and copy its value to `*result`
bool ffListPop(FFlist* list, uint32_t elementSize, void* __restrict result);

static inline void ffListInit(FFlist* list) {
    list->capacity = 0;
    list->length = 0;
    list->data = nullptr;
}

static inline void ffListInitA(FFlist* list, uint32_t elementSize, uint32_t capacity) {
    ffListInit(list);
    list->capacity = capacity;
    list->data = __builtin_expect(capacity == 0, 0) ? nullptr : (uint8_t*) malloc((size_t) capacity * elementSize);
}

[[nodiscard]] static inline FFlist ffListCreate() {
    FFlist result;
    ffListInit(&result);
    return result;
}

[[nodiscard]] static inline FFlist ffListCreateA(uint32_t elementSize, uint32_t capacity) {
    FFlist result;
    ffListInitA(&result, elementSize, capacity);
    return result;
}

[[nodiscard]] static inline void* ffListGet(const FFlist* list, uint32_t elementSize, uint32_t index) {
    assert(list->capacity > index);
    return list->data + (index * elementSize);
}

[[nodiscard]] static inline uint32_t ffListFirstIndexComp(const FFlist* list, uint32_t elementSize, void* compElement, bool (*compFunc)(const void*, const void*)) {
    for (uint32_t i = 0; i < list->length; i++) {
        if (compFunc(ffListGet(list, elementSize, i), compElement)) {
            return i;
        }
    }

    return list->length;
}

[[nodiscard]] static inline bool ffListContains(const FFlist* list, uint32_t elementSize, void* compElement, bool (*compFunc)(const void*, const void*)) {
    return ffListFirstIndexComp(list, elementSize, compElement, compFunc) != list->length;
}

static inline void ffListSort(FFlist* list, uint32_t elementSize, int (*compar)(const void*, const void*)) {
    qsort(list->data, list->length, elementSize, compar);
}

// Move the contents of `src` into `list`, and left `src` empty
static inline void ffListInitMove(FFlist* list, FFlist* src) {
    if (src) {
        list->capacity = src->capacity;
        list->length = src->length;
        list->data = src->data;
        ffListInit(src);
    } else {
        ffListInit(list);
    }
}

static inline void ffListDestroy(FFlist* list) {
    if (!list->data) {
        return;
    }

    // Avoid free-after-use. These 3 assignments are cheap so don't remove them
    list->capacity = list->length = 0;
    free(list->data);
    list->data = nullptr;
}

static inline void ffListClear(FFlist* list) {
    list->length = 0;
}

static inline void ffListReserve(FFlist* list, uint32_t elementSize, uint32_t newCapacity) {
    if (__builtin_expect(newCapacity <= list->capacity, false)) {
        return;
    }

    list->data = (uint8_t*) realloc(list->data, (size_t) newCapacity * elementSize);
    list->capacity = newCapacity;
}

static inline void* ffListAdd(FFlist* list, uint32_t elementSize) {
    if (__builtin_expect(list->length == list->capacity, false)) {
        ffListReserve(list, elementSize, list->capacity == 0 ? FF_LIST_DEFAULT_ALLOC : list->capacity * 2);
    }

    ++list->length;
    return ffListGet(list, elementSize, list->length - 1);
}

static inline void ffListRemoveAt(FFlist* list, uint32_t elementSize, uint32_t index) {
    assert(list->length > index);
    memmove(list->data + (index * elementSize), list->data + ((index + 1) * elementSize), (size_t) (list->length - index - 1) * elementSize);
    --list->length;
}

static inline void ffListInsertAt(FFlist* list, uint32_t elementSize, uint32_t index, const void* element) {
    assert(list->length >= index);
    ffListAdd(list, elementSize);
    memmove(list->data + ((index + 1) * elementSize), list->data + (index * elementSize), (size_t) (list->length - index - 1) * elementSize);
    memcpy(list->data + (index * elementSize), element, elementSize);
}

#define FF_LIST_FOR_EACH(itemType, itemVarName, listVar)                        \
    for (itemType* itemVarName = (itemType*) (listVar).data;                    \
        itemVarName - (itemType*) (listVar).data < (intptr_t) (listVar).length; \
        ++itemVarName)

#define FF_LIST_AUTO_DESTROY [[gnu::cleanup(ffListDestroy)]] FFlist

#define FF_LIST_GET(itemType, listVar, index) \
    ({                                        \
        assert((listVar).capacity > (index)); \
        (itemType*) (listVar).data + (index); \
    })

#define FF_LIST_ADD(itemType, listVar) (itemType*) ffListAdd(&(listVar), (uint32_t) sizeof(itemType))

#define FF_LIST_REMOVE_AT(itemType, listVar, index) \
    ffListRemoveAt(&(listVar), (uint32_t) sizeof(itemType), (index))

#define FF_LIST_INSERT_AT(itemType, listVar, index, pElement) \
    ffListInsertAt(&(listVar), (uint32_t) sizeof(itemType), (index), (pElement))

#define FF_LIST_FIRST(itemType, listVar) FF_LIST_GET(itemType, listVar, 0)
#define FF_LIST_LAST(itemType, listVar)                         \
    ({                                                          \
        assert((listVar).length > 0);                           \
        FF_LIST_GET(itemType, listVar, ((listVar).length - 1)); \
    })

#define FF_LIST_CONTAINS(listVar, pCompElement, compFunc)                                                                              \
    ({                                                                                                                                 \
        typedef typeof(*(pCompElement)) compElementType;                                                                               \
        typedef bool compFuncType(const compElementType*, const compElementType*);                                                     \
        static_assert(__builtin_types_compatible_p(typeof(compFunc), compFuncType), "Incompatible callback function");                 \
        ffListContains(&(listVar), (uint32_t) sizeof(*(pCompElement)), (pCompElement), (bool (*)(const void*, const void*)) compFunc); \
    })

#define FF_LIST_SHIFT(listVar, pResult) \
    ffListShift(&(listVar), (uint32_t) sizeof(*(pResult)), (pResult))
#define FF_LIST_POP(listVar, pResult) \
    ffListPop(&(listVar), (uint32_t) sizeof(*(pResult)), (pResult))
