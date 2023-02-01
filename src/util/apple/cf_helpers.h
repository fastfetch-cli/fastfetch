#pragma once

#ifndef FASTFETCH_INCLUDED_cf_helpers
#define FASTFETCH_INCLUDED_cf_helpers

#include "fastfetch.h"
#include <CoreFoundation/CoreFoundation.h>

//Return error info if failed, NULL otherwise
const char* ffCfStrGetString(CFStringRef str, FFstrbuf* result);
const char* ffCfDictGetString(CFDictionaryRef dict, CFStringRef key, FFstrbuf* result);
const char* ffCfDictGetBool(CFDictionaryRef dict, CFStringRef key, bool* result);
const char* ffCfDictGetInt(CFDictionaryRef dict, CFStringRef key, int* result);
const char* ffCfDictGetDict(CFDictionaryRef dict, CFStringRef key, CFDictionaryRef* result);

static inline CFNumberRef ffCfCreateInt(int value)
{
    return CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &value);
}

static inline void cfReleaseWrapper(void* type)
{
    if (*(CFTypeRef*) type)
        CFRelease(*(CFTypeRef*) type);
}

#define FF_CFTYPE_AUTO_RELEASE __attribute__((__cleanup__(cfReleaseWrapper)))

#endif
