#pragma once

#include "FFcheckmacros.h"

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#define FF_LIST_DEFAULT_ALLOC 16

typedef struct FFlist
{
    uint8_t* data;
    uint32_t elementSize;
    uint32_t length;
    uint32_t capacity;
} FFlist;

void* ffListAdd(FFlist* list);

// Removes the first element, and copy its value to `*result`
bool ffListShift(FFlist* list, void* result);
// Removes the last element, and copy its value to `*result`
bool ffListPop(FFlist* list, void* result);

static inline void ffListInit(FFlist* list, uint32_t elementSize)
{
    assert(elementSize > 0);
    list->elementSize = elementSize;
    list->capacity = 0;
    list->length = 0;
    list->data = NULL;
}

static inline void ffListInitA(FFlist* list, uint32_t elementSize, uint32_t capacity)
{
    ffListInit(list, elementSize);
    list->capacity = capacity;
    list->data = __builtin_expect(capacity == 0, 0) ? NULL : (uint8_t*) malloc((size_t)list->capacity * list->elementSize);
}

static inline FFlist ffListCreate(uint32_t elementSize)
{
    FFlist result;
    ffListInit(&result, elementSize);
    return result;
}

static inline void* ffListGet(const FFlist* list, uint32_t index)
{
    assert(list->capacity > index);
    return list->data + (index * list->elementSize);
}

FF_C_NODISCARD static inline uint32_t ffListFirstIndexComp(const FFlist* list, void* compElement, bool(*compFunc)(const void*, const void*))
{
    for(uint32_t i = 0; i < list->length; i++)
    {
        if(compFunc(ffListGet(list, i), compElement))
            return i;
    }

    return list->length;
}

static inline bool ffListContains(const FFlist* list, void* compElement, bool(*compFunc)(const void*, const void*))
{
    return ffListFirstIndexComp(list, compElement, compFunc) != list->length;
}

static inline void ffListSort(FFlist* list, int(*compar)(const void*, const void*))
{
    qsort(list->data, list->length, list->elementSize, compar);
}

// Move the contents of `src` into `list`, and left `src` empty
static inline void ffListInitMove(FFlist* list, FFlist* src)
{
    if (src)
    {
        list->elementSize = src->elementSize;
        list->capacity = src->capacity;
        list->length = src->length;
        list->data = src->data;
        ffListInit(src, list->elementSize);
    }
    else
    {
        ffListInit(list, 0);
    }
}

static inline void ffListDestroy(FFlist* list)
{
    if (!list->data) return;

    //Avoid free-after-use. These 3 assignments are cheap so don't remove them
    list->capacity = list->length = 0;
    free(list->data);
    list->data = NULL;
}

static inline void ffListClear(FFlist* list)
{
    list->length = 0;
}

#define FF_LIST_FOR_EACH(itemType, itemVarName, listVar) \
    assert(sizeof(itemType) == (listVar).elementSize); \
    for(itemType* itemVarName = (itemType*)(listVar).data; \
        itemVarName - (itemType*)(listVar).data < (intptr_t)(listVar).length; \
        ++itemVarName)

#define FF_LIST_AUTO_DESTROY FFlist __attribute__((__cleanup__(ffListDestroy)))

#define FF_LIST_GET(itemType, listVar, index) \
    ({ \
        assert(sizeof(itemType) == (listVar).elementSize); \
        assert((listVar).capacity > (index)); \
        (itemType*)(listVar).data + (index); \
    })

#define FF_LIST_ADD(itemType, listVar) \
    ({ \
        assert(sizeof(itemType) == (listVar).elementSize); \
        (itemType*) ffListAdd(&(listVar)); \
    })
