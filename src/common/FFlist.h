#pragma once

#include "common/attributes.h"

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#define FF_LIST_DEFAULT_ALLOC 16

typedef struct FFlist {
    uint8_t* data;
    uint32_t length;
    uint32_t capacity;
} FFlist;

void* ffListAdd(FFlist* list, uint32_t elementSize);

// Removes the first element, and copy its value to `*result`
bool ffListShift(FFlist* list, uint32_t elementSize, void* result);
// Removes the last element, and copy its value to `*result`
bool ffListPop(FFlist* list, uint32_t elementSize, void* result);

static inline void ffListInit(FFlist* list) {
    list->capacity = 0;
    list->length = 0;
    list->data = NULL;
}

static inline void ffListInitA(FFlist* list, uint32_t elementSize, uint32_t capacity) {
    ffListInit(list);
    list->capacity = capacity;
    list->data = __builtin_expect(capacity == 0, 0) ? NULL : (uint8_t*) malloc((size_t) capacity * elementSize);
}

FF_A_NODISCARD static inline FFlist ffListCreate() {
    FFlist result;
    ffListInit(&result);
    return result;
}

FF_A_NODISCARD static inline FFlist ffListCreateA(uint32_t elementSize, uint32_t capacity) {
    FFlist result;
    ffListInitA(&result, elementSize, capacity);
    return result;
}

FF_A_NODISCARD static inline void* ffListGet(const FFlist* list, uint32_t elementSize, uint32_t index) {
    assert(list->capacity > index);
    return list->data + (index * elementSize);
}

FF_A_NODISCARD static inline uint32_t ffListFirstIndexComp(const FFlist* list, uint32_t elementSize, void* compElement, bool (*compFunc)(const void*, const void*)) {
    for (uint32_t i = 0; i < list->length; i++) {
        if (compFunc(ffListGet(list, elementSize, i), compElement)) {
            return i;
        }
    }

    return list->length;
}

FF_A_NODISCARD static inline bool ffListContains(const FFlist* list, uint32_t elementSize, void* compElement, bool (*compFunc)(const void*, const void*)) {
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
    list->data = NULL;
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

#define FF_LIST_FOR_EACH(itemType, itemVarName, listVar)                        \
    for (itemType* itemVarName = (itemType*) (listVar).data;                    \
        itemVarName - (itemType*) (listVar).data < (intptr_t) (listVar).length; \
        ++itemVarName)

#define FF_LIST_AUTO_DESTROY FFlist FF_A_CLEANUP(ffListDestroy)

#define FF_LIST_GET(itemType, listVar, index) \
    ({                                        \
        assert((listVar).capacity > (index)); \
        (itemType*) (listVar).data + (index); \
    })

#define FF_LIST_ADD(itemType, listVar) (itemType*) ffListAdd(&(listVar), (uint32_t) sizeof(itemType))

#define FF_LIST_FIRST(itemType, listVar) FF_LIST_GET(itemType, listVar, 0)
#define FF_LIST_LAST(itemType, listVar)                         \
    ({                                                          \
        assert((listVar).length > 0);                           \
        FF_LIST_GET(itemType, listVar, ((listVar).length - 1)); \
    })

#define FF_LIST_CONTAINS(listVar, pCompElement, compFunc)                                                                              \
    ({                                                                                                                                 \
        typedef __typeof__(*(pCompElement)) compElementType;                                                                           \
        typedef bool compFuncType(const compElementType*, const compElementType*);                                                     \
        static_assert(__builtin_types_compatible_p(__typeof__(compFunc), compFuncType), "Incompatible callback function");             \
        ffListContains(&(listVar), (uint32_t) sizeof(*(pCompElement)), (pCompElement), (bool (*)(const void*, const void*)) compFunc); \
    })

#define FF_LIST_SHIFT(listVar, pResult) \
    ffListShift(&(listVar), (uint32_t) sizeof(*(pResult)), (pResult))
#define FF_LIST_POP(listVar, pResult) \
    ffListPop(&(listVar), (uint32_t) sizeof(*(pResult)), (pResult))
