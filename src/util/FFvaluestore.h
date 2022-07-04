#pragma once

#ifndef FASTFETCH_INCLUDED_FFVALUESTORE
#define FASTFETCH_INCLUDED_FFVALUESTORE

#include <stdbool.h>
#include <stdint.h>

typedef struct FFvaluestorePair
{
    char name[32];
    char value[1024];
} FFvaluestorePair;

typedef struct FFvaluestore
{
    FFvaluestorePair* pairs;
    uint32_t size;
    uint32_t capacity;
} FFvaluestore;

void ffValuestoreInit(FFvaluestore* vs);
void ffValuestoreSet(FFvaluestore* vs, const char* name, const char* value);
const char* ffValuestoreGet(FFvaluestore* vs, const char* name);
bool ffValuestoreContains(FFvaluestore* vs, const char* name);
void ffValuestoreDelete(FFvaluestore* vs);

#endif
