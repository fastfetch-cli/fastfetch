#include "cf_helpers.h"
#include "common/debug.h"

const char* ffCfNumGetInt64(CFTypeRef cf, int64_t* result) {
    if (CFGetTypeID(cf) == CFNumberGetTypeID()) {
        if (!CFNumberGetValue((CFNumberRef) cf, kCFNumberSInt64Type, result)) {
            FF_DEBUG("CFNumberGetValue(kCFNumberSInt64Type) failed, the number is of type 0x%08lx",
                (unsigned long) CFNumberGetType((CFNumberRef) cf));
            return "Number type is not SInt64";
        }
        return nullptr;
    } else if (CFGetTypeID(cf) == CFDataGetTypeID()) {
        if (CFDataGetLength((CFDataRef) cf) != sizeof(int64_t)) {
            FF_DEBUG("CFData holds %ld bytes, int64_t needs %zu",
                (long) CFDataGetLength((CFDataRef) cf), sizeof(int64_t));
            return "Data length is not sizeof(int64_t)";
        }
        CFDataGetBytes((CFDataRef) cf, CFRangeMake(0, sizeof(int64_t)), (uint8_t*) result);
        return nullptr;
    }

    FF_DEBUG("Expected a CFNumber or CFData, got TypeID 0x%08lx", (unsigned long) CFGetTypeID(cf));
    return "TypeID is neither 'CFNumber' nor 'CFData'";
}

const char* ffCfNumGetInt(CFTypeRef cf, int32_t* result) {
    if (CFGetTypeID(cf) == CFNumberGetTypeID()) {
        if (!CFNumberGetValue((CFNumberRef) cf, kCFNumberSInt32Type, result)) {
            FF_DEBUG("CFNumberGetValue(kCFNumberSInt32Type) failed, the number is of type 0x%08lx",
                (unsigned long) CFNumberGetType((CFNumberRef) cf));
            return "Number type is not SInt32";
        }
        return nullptr;
    } else if (CFGetTypeID(cf) == CFDataGetTypeID()) {
        if (CFDataGetLength((CFDataRef) cf) != sizeof(*result)) {
            FF_DEBUG("CFData holds %ld bytes, int32_t needs %zu",
                (long) CFDataGetLength((CFDataRef) cf), sizeof(*result));
            return "Data length is not sizeof(int32_t)";
        }
        CFDataGetBytes((CFDataRef) cf, CFRangeMake(0, sizeof(*result)), (uint8_t*) result);
        return nullptr;
    }

    FF_DEBUG("Expected a CFNumber or CFData, got TypeID 0x%08lx", (unsigned long) CFGetTypeID(cf));
    return "TypeID is neither 'CFNumber' nor 'CFData'";
}

const char* ffCfNumGetDouble(CFTypeRef cf, double* result) {
    if (CFGetTypeID(cf) == CFNumberGetTypeID()) {
        if (!CFNumberGetValue((CFNumberRef) cf, kCFNumberDoubleType, result) &&
            !CFNumberGetValue((CFNumberRef) cf, kCFNumberFloatType, result)) {
            FF_DEBUG("CFNumber is of type 0x%08lx, neither Double nor Float",
                (unsigned long) CFNumberGetType((CFNumberRef) cf));
            return "Number type is not Double or Float";
        }
        return nullptr;
    }

    FF_DEBUG("Expected a CFNumber, got TypeID 0x%08lx", (unsigned long) CFGetTypeID(cf));
    return "TypeID is neither 'CFNumber'";
}

const char* ffCfDateGetEpoch(CFTypeRef cf, uint64_t* result) {
    if (CFGetTypeID(cf) != CFDateGetTypeID()) {
        FF_DEBUG("Expected a CFDate, got TypeID 0x%08lx", (unsigned long) CFGetTypeID(cf));
        return "TypeID is not 'CFDate'";
    }

    CFAbsoluteTime absTime = CFDateGetAbsoluteTime((CFDateRef) cf);
    // Convert from seconds to milliseconds and add the difference between 1970 and 2001 in milliseconds
    *result = (uint64_t) ((absTime + 978307200 /*kCFAbsoluteTimeIntervalSince1970*/) * 1000);
    return nullptr;
}

const char* ffCfStrGetString(CFTypeRef cf, FFstrbuf* result) {
    ffStrbufClear(result);
    if (!cf) {
        return nullptr;
    }

    if (CFGetTypeID(cf) == CFStringGetTypeID()) {
        CFStringRef cfStr = (CFStringRef) cf;

        const char* cstr = CFStringGetCStringPtr(cfStr, kCFStringEncodingUTF8);
        if (cstr) {
            ffStrbufSetS(result, cstr);
        } else {
            uint32_t length = (uint32_t) CFStringGetLength(cfStr);
            if (length == 0) {
                return nullptr;
            }
            ffStrbufEnsureFixedLengthFree(result, (uint32_t) CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8));
            if (!CFStringGetCString(cfStr, result->chars, result->allocated, kCFStringEncodingUTF8)) {
                FF_DEBUG("CFStringGetCString() failed for a %u character string", (unsigned) length);
                return "CFStringGetCString() failed";
            }
            // CFStringGetCString ensures the buffer is NUL terminated
            // https://developer.apple.com/documentation/corefoundation/1542721-cfstringgetcstring
            result->length = (uint32_t) strnlen(result->chars, (uint32_t) result->allocated);
        }
    } else if (CFGetTypeID(cf) == CFDataGetTypeID()) {
        CFDataRef cfData = (CFDataRef) cf;
        uint32_t length = (uint32_t) CFDataGetLength(cfData);
        if (length == 0) {
            return nullptr;
        }
        ffStrbufEnsureFixedLengthFree(result, length + 1);
        CFDataGetBytes(cfData, CFRangeMake(0, length), (uint8_t*) result->chars);
        result->length = (uint32_t) strnlen(result->chars, length);
        result->chars[result->length] = '\0';
    } else {
        FF_DEBUG("Expected a CFString or CFData, got TypeID 0x%08lx", (unsigned long) CFGetTypeID(cf));
        return "TypeID is neither 'CFString' nor 'CFData'";
    }

    return nullptr;
}

const char* ffCfDataGetDataAsString(CFTypeRef cf, FFstrbuf* result) {
    ffStrbufClear(result);
    if (!cf) {
        return nullptr;
    }

    if (CFGetTypeID(cf) == CFDataGetTypeID()) {
        CFDataRef cfData = (CFDataRef) cf;
        uint32_t length = (uint32_t) CFDataGetLength(cfData);
        if (length == 0) {
            return nullptr;
        }
        ffStrbufEnsureFixedLengthFree(result, length + 1);
        CFDataGetBytes(cfData, CFRangeMake(0, length), (uint8_t*) result->chars);
        result->length = length;
        result->chars[result->length] = '\0';
    } else {
        FF_DEBUG("Expected a CFData, got TypeID 0x%08lx", (unsigned long) CFGetTypeID(cf));
        return "TypeID is not 'CFData'";
    }

    return nullptr;
}

const char* ffCfDictGetString(CFDictionaryRef dict, CFStringRef key, FFstrbuf* result) {
    CFTypeRef cf = (CFTypeRef) CFDictionaryGetValue(dict, key);
    if (cf == nullptr) {
        FF_DEBUG("The dictionary has no value for the key");
        return "CFDictionaryGetValue() failed";
    }

    return ffCfStrGetString(cf, result);
}

const char* ffCfDictGetDataAsString(CFDictionaryRef dict, CFStringRef key, FFstrbuf* result) {
    CFTypeRef cf = (CFTypeRef) CFDictionaryGetValue(dict, key);
    if (cf == nullptr) {
        FF_DEBUG("The dictionary has no value for the key");
        return "CFDictionaryGetValue() failed";
    }

    return ffCfDataGetDataAsString(cf, result);
}

const char* ffCfDictGetBool(CFDictionaryRef dict, CFStringRef key, bool* result) {
    CFBooleanRef cf = (CFBooleanRef) CFDictionaryGetValue(dict, key);
    if (cf == nullptr) {
        FF_DEBUG("The dictionary has no value for the key");
        return "CFDictionaryGetValue() failed";
    }

    if (CFGetTypeID(cf) != CFBooleanGetTypeID()) {
        FF_DEBUG("Expected a CFBoolean, got TypeID 0x%08lx", (unsigned long) CFGetTypeID(cf));
        return "TypeID is not 'CFBoolean'";
    }

    *result = CFBooleanGetValue(cf);
    return nullptr;
}

const char* ffCfDictGetInt(CFDictionaryRef dict, CFStringRef key, int* result) {
    CFTypeRef cf = (CFTypeRef) CFDictionaryGetValue(dict, key);
    if (cf == nullptr) {
        FF_DEBUG("The dictionary has no value for the key");
        return "CFDictionaryGetValue() failed";
    }

    return ffCfNumGetInt(cf, result);
}

const char* ffCfDictGetInt64(CFDictionaryRef dict, CFStringRef key, int64_t* result) {
    CFTypeRef cf = (CFTypeRef) CFDictionaryGetValue(dict, key);
    if (cf == nullptr) {
        FF_DEBUG("The dictionary has no value for the key");
        return "CFDictionaryGetValue() failed";
    }

    return ffCfNumGetInt64(cf, result);
}

const char* ffCfDictGetDouble(CFDictionaryRef dict, CFStringRef key, double* result) {
    CFTypeRef cf = (CFTypeRef) CFDictionaryGetValue(dict, key);
    if (cf == nullptr) {
        FF_DEBUG("The dictionary has no value for the key");
        return "CFDictionaryGetValue() failed";
    }

    return ffCfNumGetDouble(cf, result);
}

const char* ffCfDictGetData(CFDictionaryRef dict, CFStringRef key, uint32_t offset, uint32_t size, uint8_t* result, uint32_t* length) {
    CFTypeRef cf = (CFTypeRef) CFDictionaryGetValue(dict, key);
    if (cf == nullptr) {
        FF_DEBUG("The dictionary has no value for the key");
        return "CFDictionaryGetValue() failed";
    }

    if (CFGetTypeID(cf) != CFDataGetTypeID()) {
        FF_DEBUG("Expected a CFData, got TypeID 0x%08lx", (unsigned long) CFGetTypeID(cf));
        return "TypeID is not 'CFData'";
    }

    CFIndex trueLength = CFDataGetLength((CFDataRef) cf);

    if (trueLength < offset + size) {
        FF_DEBUG("The CFData holds %ld bytes, the caller asked for [%u, %u)",
            (long) trueLength, offset, offset + size);
        return "Data length is less than offset + size";
    }

    if (length) {
        *length = (uint32_t) trueLength;
    }

    CFDataGetBytes((CFDataRef) cf, CFRangeMake(offset, size), result);
    return nullptr;
}

const char* ffCfDictGetDict(CFDictionaryRef dict, CFStringRef key, CFDictionaryRef* result) {
    CFDictionaryRef cf = (CFDictionaryRef) CFDictionaryGetValue(dict, key);
    if (cf == nullptr || CFGetTypeID(cf) != CFDictionaryGetTypeID()) {
        FF_DEBUG("Expected a CFDictionary, got %s",
            cf == nullptr ? "no value at all" : "a value of another type");
        return "TypeID is not 'CFDictionary'";
    }

    *result = cf;
    return nullptr;
}

const char* ffCfDictGetDateAsEpoch(CFDictionaryRef dict, CFStringRef key, uint64_t* result) {
    CFTypeRef cf = (CFTypeRef) CFDictionaryGetValue(dict, key);
    if (cf == nullptr) {
        FF_DEBUG("The dictionary has no value for the key");
        return "CFDictionaryGetValue() failed";
    }

    return ffCfDateGetEpoch(cf, result);
}
