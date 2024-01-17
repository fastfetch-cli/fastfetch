#include "smbiosHelper.h"
#include "common/io/io.h"
#include "util/unused.h"

bool ffIsSmbiosValueSet(FFstrbuf* value)
{
    ffStrbufTrimRightSpace(value);
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
        !ffStrbufIgnCaseEqualS(value, "No Enclosure") &&
        !ffStrbufIgnCaseEqualS(value, "Chassis Version") &&
        !ffStrbufIgnCaseEqualS(value, "All Series") &&
        !ffStrbufIgnCaseEqualS(value, "N/A")
    ;
}

#ifdef __linux__
bool ffGetSmbiosValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer)
{
    if (ffReadFileBuffer(devicesPath, buffer))
    {
        ffStrbufTrimRightSpace(buffer);
        if(ffIsSmbiosValueSet(buffer))
            return true;
    }

    if (ffReadFileBuffer(classPath, buffer))
    {
        ffStrbufTrimRightSpace(buffer);
        if(ffIsSmbiosValueSet(buffer))
            return true;
    }

    ffStrbufClear(buffer);
    return false;
}
#elif defined(_WIN32)
#include <windows.h>

#pragma GCC diagnostic ignored "-Wmultichar"

const FFSmbiosHeader* ffSmbiosNextEntry(const FFSmbiosHeader* header)
{
    const char* p = ((const char*) header) + header->Length;
    if (*p)
    {
        do
            p += strlen(p) + 1;
        while (*p);
    }
    else // The terminator is always double 0 even if there is no string
        p ++;

    return (const FFSmbiosHeader*) (p + 1);
}

const FFSmbiosHeaderTable* ffGetSmbiosHeaderTable()
{
    static FFRawSmbiosData* buffer;
    static FFSmbiosHeaderTable table;

    if (!buffer)
    {
        const DWORD signature = 'RSMB';
        uint32_t bufSize = GetSystemFirmwareTable(signature, 0, NULL, 0);
        assert(bufSize > sizeof(FFRawSmbiosData));
        buffer = (FFRawSmbiosData*) malloc(bufSize);
        assert(buffer);
        FF_MAYBE_UNUSED uint32_t resultSize = GetSystemFirmwareTable(signature, 0, buffer, bufSize);
        assert(resultSize == bufSize);

        for (
            const FFSmbiosHeader* header = (const FFSmbiosHeader*) buffer->SMBIOSTableData;
            (const uint8_t*) header < buffer->SMBIOSTableData + buffer->Length;
            header = ffSmbiosNextEntry(header)
        )
        {
            if (header->Type < FF_SMBIOS_TYPE_END_OF_TABLE)
            {
                if (!table[header->Type])
                    table[header->Type] = header;
            }
            else if (header->Type == FF_SMBIOS_TYPE_END_OF_TABLE)
                break;
        }
    }

    return &table;
}
#endif
