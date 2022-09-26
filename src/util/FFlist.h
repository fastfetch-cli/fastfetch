#pragma once

#ifndef FF_INCLUDED_FFLIST
#define FF_INCLUDED_FFLIST

#include "FFcheckmacros.h"

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

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

#endif
