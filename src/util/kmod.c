#include "kmod.h"

#if __linux__
#include "common/io/io.h"

bool ffKmodLoaded(const char* modName)
{
    static FFstrbuf modules;
    if (modules.chars == NULL)
    {
        ffStrbufInitS(&modules, "\n");
        ffAppendFileBuffer("/proc/modules", &modules);
    }

    if (modules.length == 0) return false;

    uint32_t len = (uint32_t) strlen(modName);
    if (len > 250) return false;

    char temp[256];
    temp[0] = '\n';
    memcpy(temp + 1, modName, len);
    temp[1 + len] = ' ';
    return memmem(modules.chars, modules.length, temp, len + 2) != NULL;
}
#elif __FreeBSD__
#include <sys/param.h>
#include <sys/module.h>

bool ffKmodLoaded(const char* modName)
{
    return modfind(modName) >= 0;
}
#elif __APPLE__
#include "util/apple/cf_helpers.h"
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
#else
bool ffKmodLoaded(FF_MAYBE_UNUSED const char* modName)
{
    return true; // Don't generate kernel module related errors
}
#endif
