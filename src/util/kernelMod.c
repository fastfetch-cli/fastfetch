#include "kernelMod.h"

#if __linux__
#include "common/io/io.h"

bool ffKernelModLoaded(const char* modName)
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

bool ffKernelModLoaded(const char* modName)
{
    return modfind(modName) >= 0;
}
#else
bool ffKernelModLoaded(FF_MAYBE_UNUSED const char* modName)
{
    return true; // Don't generate kernel module related errors
}
#endif
