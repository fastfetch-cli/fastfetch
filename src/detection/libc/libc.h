#pragma once

#ifndef FF_INCLUDED_detection_libc_libc
#define FF_INCLUDED_detection_libc_libc

#include "fastfetch.h"

typedef struct FFLibcResult
{
    const char* name;
    const char* version;
} FFLibcResult;

const char* ffDetectLibc(FFLibcResult* result);

#endif
