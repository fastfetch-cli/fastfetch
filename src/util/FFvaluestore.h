#pragma once

#ifndef FASTFETCH_INCLUDED_FFVALUESTORE
#define FASTFETCH_INCLUDED_FFVALUESTORE

#include "FFlist.h"
#include "FFcheckmacros.h"

typedef FFlist FFvaluestore;

void ffValuestoreInit(FFvaluestore* vs, uint32_t valueSize);
FF_C_NODISCARD void* ffValuestoreGet(FFvaluestore* vs, const char* key);
FF_C_NODISCARD void* ffValuestoreSet(FFvaluestore* vs, const char* key, bool* created); //created may be NULL
void ffValuestoreDestroy(FFvaluestore* vs);

#endif
