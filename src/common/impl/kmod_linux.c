#include "common/kmod.h"
#include "common/io.h"

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
