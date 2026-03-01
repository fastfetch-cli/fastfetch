#include "common/kmod.h"
#include "common/windows/nt.h"
#include "common/mallocHelper.h"
#include "common/stringUtils.h"

bool ffKmodLoaded(FF_MAYBE_UNUSED const char* modName)
{
    ULONG bufferSize = 0;
    NtQuerySystemInformation(SystemModuleInformation, NULL, 0, &bufferSize);
    if (bufferSize == 0)
        return true; // ignore errors

    FF_AUTO_FREE RTL_PROCESS_MODULES* buffer = malloc(bufferSize);

    if (!NT_SUCCESS(NtQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, &bufferSize)))
        return true; // ignore errors

    for (ULONG i = 0; i < buffer->NumberOfModules; i++)
    {
        const char* name = (const char*) buffer->Modules[i].FullPathName + buffer->Modules[i].OffsetToFileName;

        if (ffStrEqualsIgnCase(name, modName))
            return true;
    }

    return false;
}
