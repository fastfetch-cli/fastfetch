#pragma once

#ifndef FF_INCLUDED_FFLIST
#define FF_INCLUDED_FFLIST

#include <stdbool.h>
#include <stdint.h>

#define FF_LIST_DEFAULT_ALLOC 16

typedef struct FFlist
{
    char* data;
    uint32_t elementSize;
    uint32_t length;
    uint32_t capacity;
} FFlist;

void ffListInit(FFlist* list, uint32_t elementSize);
void ffListInitA(FFlist* list, uint32_t elementSize, uint32_t capacity);

void* ffListGet(FFlist* list, uint32_t index);

void* ffListAdd(FFlist* list);

uint32_t ffListFirstIndexComp(FFlist* list, void* compElement, bool(*compFunc)(const void*, const void*));

void ffListDestroy(FFlist* list);

#endif
