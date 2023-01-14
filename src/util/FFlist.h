#pragma once

#ifndef FF_INCLUDED_FFLIST
#define FF_INCLUDED_FFLIST

#include "FFcheckmacros.h"

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#define FF_LIST_DEFAULT_ALLOC 16

typedef struct FFlist
{
    char* data;
    uint32_t elementSize;
    uint32_t length;
    uint32_t capacity;
} FFlist;

void ffListInitA(FFlist* list, uint32_t elementSize, uint32_t capacity);

void* ffListAdd(FFlist* list);

FF_C_NODISCARD uint32_t ffListFirstIndexComp(const FFlist* list, void* compElement, bool(*compFunc)(const void*, const void*));

// Removes the first element, and copy its value to `*result`
bool ffListShift(FFlist* list, void* result);
// Removes the last element, and copy its value to `*result`
bool ffListPop(FFlist* list, void* result);

void ffListDestroy(FFlist* list);

static inline void ffListInit(FFlist* list, uint32_t elementSize)
{
    assert(elementSize > 0);
    ffListInitA(list, elementSize, 0);
}

static inline void* ffListGet(const FFlist* list, uint32_t index)
{
    assert(list->capacity > index);
    return list->data + (index * list->elementSize);
}

static inline void ffListSort(FFlist* list, int(*compar)(const void*, const void*))
{
    qsort(list->data, list->length, list->elementSize, compar);
}

#define FF_LIST_FOR_EACH(itemType, itemVarName, listVar) \
    assert(sizeof(itemType) == (listVar).elementSize); \
    for(itemType* itemVarName = (itemType*)(listVar).data; \
        itemVarName - (itemType*)(listVar).data < (listVar).length; \
        ++itemVarName)

#define FF_LIST_AUTO_DESTROY FFlist __attribute__((__cleanup__(ffListDestroy)))

#endif
