#pragma once

#ifndef FF_INCLUDED_FFLIST
#define FF_INCLUDED_FFLIST

#include <stdint.h>

#define FF_LIST_DEFAULT_ALLOC 16

typedef struct FFlist
{
    void* data;
    uint32_t elementSize;
    uint32_t length;
    uint32_t capacity;
} FFlist;

void ffListInit(FFlist* list, uint32_t elementSize);
void ffListInitA(FFlist* list, uint32_t elementSize, uint32_t capacity);

void* ffListGet(FFlist* list, uint32_t index);

void* ffListAdd(FFlist* list);

void ffListDestroy(FFlist* list);

#endif
