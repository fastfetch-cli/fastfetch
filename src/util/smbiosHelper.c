#include "smbiosHelper.h"
#include "common/io/io.h"

bool ffIsSmbiosValueSet(FFstrbuf* value)
{
    return
        value->length > 0 &&
        !ffStrbufStartsWithIgnCaseS(value, "To be filled") &&
        !ffStrbufStartsWithIgnCaseS(value, "To be set") &&
        !ffStrbufStartsWithIgnCaseS(value, "OEM") &&
        !ffStrbufStartsWithIgnCaseS(value, "O.E.M.") &&
        !ffStrbufIgnCaseEqualS(value, "None") &&
        !ffStrbufIgnCaseEqualS(value, "System Product") &&
        !ffStrbufIgnCaseEqualS(value, "System Product Name") &&
        !ffStrbufIgnCaseEqualS(value, "System Product Version") &&
        !ffStrbufIgnCaseEqualS(value, "System Name") &&
        !ffStrbufIgnCaseEqualS(value, "System Version") &&
        !ffStrbufIgnCaseEqualS(value, "Default string") &&
        !ffStrbufIgnCaseEqualS(value, "Undefined") &&
        !ffStrbufIgnCaseEqualS(value, "Not Specified") &&
        !ffStrbufIgnCaseEqualS(value, "Not Applicable") &&
        !ffStrbufIgnCaseEqualS(value, "Not Defined") &&
        !ffStrbufIgnCaseEqualS(value, "Not Available") &&
        !ffStrbufIgnCaseEqualS(value, "INVALID") &&
        !ffStrbufIgnCaseEqualS(value, "Type1ProductConfigId") &&
        !ffStrbufIgnCaseEqualS(value, "All Series") &&
        !ffStrbufIgnCaseEqualS(value, "N/A")
    ;
}

#ifdef __linux__
void ffGetSmbiosValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer)
{
    if (ffReadFileBuffer(devicesPath, buffer))
    {
        ffStrbufTrimRightSpace(buffer);
        if(ffIsSmbiosValueSet(buffer))
            return;
    }

    if (ffReadFileBuffer(classPath, buffer))
    {
        ffStrbufTrimRightSpace(buffer);
        if(ffIsSmbiosValueSet(buffer))
            return;
    }

    ffStrbufClear(buffer);
}
#elif defined(_WIN32)
#include <windows.h>

#pragma GCC diagnostic ignored "-Wmultichar"

const FFRawSmbiosData* ffGetSmbiosData()
{
    static FFRawSmbiosData* buffer;
    if (!buffer)
    {
        const DWORD signature = 'RSMB';
        uint32_t bufSize = GetSystemFirmwareTable(signature, 0, NULL, 0);
        assert(bufSize > sizeof(FFRawSmbiosData));
        buffer = (FFRawSmbiosData*) malloc(bufSize);
        GetSystemFirmwareTable(signature, 0, buffer, bufSize);
    }
    return buffer;
}
#endif
