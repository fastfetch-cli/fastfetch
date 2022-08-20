#pragma once

#ifndef FASTFETCH_INCLUDED_FFVALUESTORE
#define FASTFETCH_INCLUDED_FFVALUESTORE

#include "util/FFlist.h"

typedef FFlist FFvaluestore;

void ffValuestoreInit(FFvaluestore* vs, uint32_t valueSize);
void* ffValuestoreGet(FFvaluestore* vs, const char* key);
void* ffValuestoreSet(FFvaluestore* vs, const char* key, bool* created); //created may be NULL
void ffValuestoreDestroy(FFvaluestore* vs);

#endif
