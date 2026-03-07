#include "common/kmod.h"
#include "common/apple/cf_helpers.h"
#include <IOKit/kext/KextManager.h>
#include <CoreFoundation/CoreFoundation.h>

bool ffKmodLoaded(const char* modName)
{
    FF_CFTYPE_AUTO_RELEASE CFStringRef name = CFStringCreateWithCString(kCFAllocatorDefault, modName, kCFStringEncodingUTF8);
    FF_CFTYPE_AUTO_RELEASE CFArrayRef identifiers = CFArrayCreate(kCFAllocatorDefault, (const void**) &name, 1, &kCFTypeArrayCallBacks);
    FF_CFTYPE_AUTO_RELEASE CFArrayRef keys = CFArrayCreate(kCFAllocatorDefault, NULL, 0, NULL);
    FF_CFTYPE_AUTO_RELEASE CFDictionaryRef kextInfo = KextManagerCopyLoadedKextInfo(identifiers, keys);
    return CFDictionaryContainsKey(kextInfo, name);
}
