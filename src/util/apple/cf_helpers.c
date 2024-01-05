#include "cf_helpers.h"

const char* ffCfNumGetInt64(CFTypeRef cf, int64_t* result)
{
    if(CFGetTypeID(cf) == CFNumberGetTypeID())
    {
        if(!CFNumberGetValue((CFNumberRef)cf, kCFNumberSInt64Type, result))
            return "Number type is not SInt64";
        return NULL;
    }
    else if(CFGetTypeID(cf) == CFDataGetTypeID())
    {
        if(CFDataGetLength((CFDataRef)cf) != sizeof(int64_t))
            return "Data length is not sizeof(int64_t)";
        CFDataGetBytes((CFDataRef)cf, CFRangeMake(0, sizeof(int64_t)), (uint8_t*)result);
        return NULL;
    }

    return "TypeID is neither 'CFNumber' nor 'CFData'";
}

const char* ffCfStrGetString(CFTypeRef cf, FFstrbuf* result)
{
    if (!cf)
    {
        ffStrbufClear(result);
        return NULL;
    }

    if (CFGetTypeID(cf) == CFStringGetTypeID())
    {
        CFStringRef cfStr = (CFStringRef)cf;
        uint32_t length = (uint32_t)CFStringGetLength(cfStr);
        //CFString stores UTF16 characters, therefore may require larger buffer to convert to UTF8 string
        ffStrbufEnsureFree(result, length * 2);
        if (!CFStringGetCString(cfStr, result->chars, result->allocated, kCFStringEncodingUTF8))
        {
            ffStrbufEnsureFree(result, length * 4);
            if(!CFStringGetCString(cfStr, result->chars, result->allocated, kCFStringEncodingUTF8))
                return "CFStringGetCString() failed";
        }
        // CFStringGetCString ensures the buffer is NUL terminated
        // https://developer.apple.com/documentation/corefoundation/1542721-cfstringgetcstring
        result->length = (uint32_t) strnlen(result->chars, (uint32_t)result->allocated);
    }
    else if (CFGetTypeID(cf) == CFDataGetTypeID())
    {
        CFDataRef cfData = (CFDataRef)cf;
        uint32_t length = (uint32_t)CFDataGetLength(cfData);
        ffStrbufEnsureFree(result, length + 1);
        CFDataGetBytes(cfData, CFRangeMake(0, length), (uint8_t*)result->chars);
        result->length = (uint32_t)strnlen(result->chars, length);
        result->chars[result->length] = '\0';
    }
    else
        return "TypeID is neither 'CFString' nor 'CFData'";

    return NULL;
}

const char* ffCfDictGetString(CFDictionaryRef dict, CFStringRef key, FFstrbuf* result)
{
    CFTypeRef cf = (CFTypeRef)CFDictionaryGetValue(dict, key);
    if(cf == NULL)
        return "CFDictionaryGetValue() failed";

    return ffCfStrGetString(cf, result);
}

const char* ffCfDictGetBool(CFDictionaryRef dict, CFStringRef key, bool* result)
{
    CFBooleanRef cf = (CFBooleanRef)CFDictionaryGetValue(dict, key);
    if(cf == NULL)
        return "CFDictionaryGetValue() failed";

    if(CFGetTypeID(cf) != CFBooleanGetTypeID())
        return "TypeID is not 'CFBoolean'";

    *result = CFBooleanGetValue(cf);
    return NULL;
}

const char* ffCfDictGetInt(CFDictionaryRef dict, CFStringRef key, int* result)
{
    CFTypeRef cf = (CFTypeRef)CFDictionaryGetValue(dict, key);
    if(cf == NULL)
        return "CFDictionaryGetValue() failed";

    if(CFGetTypeID(cf) == CFNumberGetTypeID())
    {
        if(!CFNumberGetValue((CFNumberRef)cf, kCFNumberSInt32Type, result))
            return "Number type is not SInt32";
        return NULL;
    }
    else if(CFGetTypeID(cf) == CFDataGetTypeID())
    {
        if(CFDataGetLength((CFDataRef)cf) != sizeof(int))
            return "Data length is not sizeof(int)";
        CFDataGetBytes((CFDataRef)cf, CFRangeMake(0, sizeof(int)), (uint8_t*)result);
        return NULL;
    }

    return "TypeID is neither 'CFNumber' nor 'CFData'";
}

const char* ffCfDictGetInt64(CFDictionaryRef dict, CFStringRef key, int64_t* result)
{
    CFTypeRef cf = (CFTypeRef)CFDictionaryGetValue(dict, key);
    if(cf == NULL)
        return "CFDictionaryGetValue() failed";

    return ffCfNumGetInt64(cf, result);
}

const char* ffCfDictGetData(CFDictionaryRef dict, CFStringRef key, uint32_t offset, uint32_t size, uint8_t* result, uint32_t* length)
{
    CFTypeRef cf = (CFTypeRef)CFDictionaryGetValue(dict, key);
    if(cf == NULL)
        return "CFDictionaryGetValue() failed";

    if(CFGetTypeID(cf) != CFDataGetTypeID())
        return "TypeID is not 'CFData'";

    CFIndex trueLength = CFDataGetLength((CFDataRef)cf);

    if(trueLength < offset + size)
        return "Data length is less than offset + size";

    if(length)
        *length = (uint32_t) trueLength;

    CFDataGetBytes((CFDataRef)cf, CFRangeMake(offset, size), result);
    return NULL;
}

const char* ffCfDictGetDict(CFDictionaryRef dict, CFStringRef key, CFDictionaryRef* result)
{
    CFDictionaryRef cf = (CFDictionaryRef)CFDictionaryGetValue(dict, key);
    if (cf == NULL || CFGetTypeID(cf) != CFDictionaryGetTypeID())
        return "TypeID is not 'CFDictionary'";

    *result = cf;
    return NULL;
}
