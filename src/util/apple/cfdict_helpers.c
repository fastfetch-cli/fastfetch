#include "cfdict_helpers.h"

const void* ffCfDictGetValue(CFMutableDictionaryRef dict, const char* key)
{
    CFStringRef cfKey = CFStringCreateWithCStringNoCopy(NULL, key, kCFStringEncodingASCII, kCFAllocatorNull);
    return CFDictionaryGetValue(dict, cfKey);
}

bool ffCfDictGetString(CFMutableDictionaryRef dict, const char* key, FFstrbuf* result)
{
    CFStringRef cf = (CFStringRef)ffCfDictGetValue(dict, key);
    if(cf == NULL || CFGetTypeID(cf) != CFStringGetTypeID())
        return false;

    uint32_t length = (uint32_t)CFStringGetLength(cf);
    ffStrbufEnsureFree(result, length + 1);
    if(CFStringGetCString(cf, result->chars, length + 1, kCFStringEncodingASCII))
    {
        result->length = length;
        // CFStringGetCString ensures the buffer is NUL terminated
        // https://developer.apple.com/documentation/corefoundation/1542721-cfstringgetcstring
    }
    return true;
}

bool ffCfDictGetBool(CFMutableDictionaryRef dict, const char* key, bool* result)
{
    CFBooleanRef cf = (CFBooleanRef)ffCfDictGetValue(dict, key);
    if(cf == NULL || CFGetTypeID(cf) != CFBooleanGetTypeID())
        return false;

    *result = CFBooleanGetValue(cf);
    return true;
}

bool ffCfDictGetInt(CFMutableDictionaryRef dict, const char* key, int* result)
{
    CFNumberRef cf = (CFNumberRef)ffCfDictGetValue(dict, key);
    if (cf == NULL || CFGetTypeID(cf) != CFNumberGetTypeID())
        return false;

    if(!CFNumberGetValue(cf, kCFNumberSInt32Type, result))
        return false;
    return true;
}
