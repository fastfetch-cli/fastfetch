#include "cfdict_helpers.h"

bool ffCfDictGetString(CFDictionaryRef dict, CFStringRef key, FFstrbuf* result)
{
    CFTypeRef cf = (CFTypeRef)CFDictionaryGetValue(dict, key);
    if(cf == NULL)
        return false;

    if(CFGetTypeID(cf) == CFStringGetTypeID())
    {
        CFStringRef cfStr = (CFStringRef)cf;
        uint32_t length = (uint32_t)CFStringGetLength(cfStr);
        //CFString stores UTF16 characters, therefore may require larger buffer to convert to UTF8 string
        ffStrbufEnsureFree(result, length * 2);
        if(CFStringGetCString(cfStr, result->chars, result->allocated, kCFStringEncodingUTF8))
        {
            // CFStringGetCString ensures the buffer is NUL terminated
            // https://developer.apple.com/documentation/corefoundation/1542721-cfstringgetcstring
            result->length = strnlen(result->chars, (uint32_t)result->allocated);
        }
    }
    else if(CFGetTypeID(cf) == CFDataGetTypeID())
    {
        CFDataRef cfData = (CFDataRef)cf;
        uint32_t length = (uint32_t)CFDataGetLength(cfData);
        ffStrbufEnsureFree(result, length + 1);
        CFDataGetBytes(cfData, CFRangeMake(0, length), (uint8_t*)result->chars);
        result->length = (uint32_t)strnlen(result->chars, length);
        result->chars[result->length] = '\0';
    }
    else
    {
        return false;
    }
    return true;
}

bool ffCfDictGetBool(CFDictionaryRef dict, CFStringRef key, bool* result)
{
    CFBooleanRef cf = (CFBooleanRef)CFDictionaryGetValue(dict, key);
    if(cf == NULL || CFGetTypeID(cf) != CFBooleanGetTypeID())
        return false;

    *result = CFBooleanGetValue(cf);
    return true;
}

bool ffCfDictGetInt(CFDictionaryRef dict, CFStringRef key, int* result)
{
    CFNumberRef cf = (CFNumberRef)CFDictionaryGetValue(dict, key);
    if (cf == NULL || CFGetTypeID(cf) != CFNumberGetTypeID())
        return false;

    if(!CFNumberGetValue(cf, kCFNumberSInt32Type, result))
        return false;
    return true;
}
