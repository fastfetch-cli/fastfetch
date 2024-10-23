#pragma once

#include "fastfetch.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

//Return error info if failed, NULL otherwise
const char* ffCfStrGetString(CFTypeRef cf, FFstrbuf* result);
const char* ffCfNumGetInt(CFTypeRef cf, int32_t* result);
const char* ffCfNumGetInt64(CFTypeRef cf, int64_t* result);
const char* ffCfDictGetString(CFDictionaryRef dict, CFStringRef key, FFstrbuf* result);
const char* ffCfDictGetBool(CFDictionaryRef dict, CFStringRef key, bool* result);
const char* ffCfDictGetInt(CFDictionaryRef dict, CFStringRef key, int* result);
const char* ffCfDictGetInt64(CFDictionaryRef dict, CFStringRef key, int64_t* result);
const char* ffCfDictGetData(CFDictionaryRef dict, CFStringRef key, uint32_t offset, uint32_t size, uint8_t* result, uint32_t* length);
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

static inline void wrapIoObjectRelease(io_service_t* service)
{
    assert(service);
    if (*service)
        IOObjectRelease(*service);
}
#define FF_IOOBJECT_AUTO_RELEASE __attribute__((__cleanup__(wrapIoObjectRelease)))
