#include "smbiosHelper.h"
#include "common/io/io.h"
#include "util/unused.h"
#include "util/mallocHelper.h"

bool ffIsSmbiosValueSet(FFstrbuf* value)
{
    ffStrbufTrimRightSpace(value);
    return
        value->length > 0 &&
        !ffStrbufStartsWithIgnCaseS(value, "To be filled") &&
        !ffStrbufStartsWithIgnCaseS(value, "To be set") &&
        !ffStrbufStartsWithIgnCaseS(value, "OEM") &&
        !ffStrbufStartsWithIgnCaseS(value, "O.E.M.") &&
        !ffStrbufStartsWithIgnCaseS(value, "System Product") &&
        !ffStrbufIgnCaseEqualS(value, "None") &&
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
        !ffStrbufIgnCaseEqualS(value, "TBD by OEM") &&
        !ffStrbufIgnCaseEqualS(value, "No Enclosure") &&
        !ffStrbufIgnCaseEqualS(value, "Chassis Version") &&
        !ffStrbufIgnCaseEqualS(value, "All Series") &&
        !ffStrbufIgnCaseEqualS(value, "N/A") &&
        !ffStrbufIgnCaseEqualS(value, "0x0000")
    ;
}

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

#if defined(__linux__) || defined(__FreeBSD__) || defined(__sun)
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stddef.h>

#ifdef __linux__
    #include "common/properties.h"
#elif defined(__FreeBSD__)
    #include "common/settings.h"
    #define loff_t off_t // FreeBSD doesn't have loff_t
#elif defined(__sun)
    #define loff_t off_t
#endif

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

typedef struct FFSmbios20EntryPoint
{
    uint8_t AnchorString[4];
    uint8_t EntryPointStructureChecksum;
    uint8_t EntryPointLength;
    uint8_t SmbiosMajorVersion;
    uint8_t SmbiosMinorVersion;
    uint16_t MaximumStructureSize;
    uint8_t EntryPointRevision;
    uint8_t FormattedArea[5];
    uint8_t IntermediateAnchorString[5];
    uint8_t IntermediateChecksum;
    uint16_t StructureTableLength;
    uint32_t StructureTableAddress;
    uint16_t NumberOfSmbiosStructures;
    uint8_t SmbiosBcdRevision;
} __attribute__((__packed__)) FFSmbios20EntryPoint;
static_assert(offsetof(FFSmbios20EntryPoint, SmbiosBcdRevision) == 0x1E,
    "FFSmbios30EntryPoint: Wrong struct alignment");

typedef struct FFSmbios30EntryPoint
{
    uint8_t AnchorString[5];
    uint8_t EntryPointStructureChecksum;
    uint8_t EntryPointLength;
    uint8_t SmbiosMajorVersion;
    uint8_t SmbiosMinorVersion;
    uint8_t SmbiosDocrev;
    uint8_t EntryPointRevision;
    uint8_t Reversed;
    uint32_t StructureTableMaximumSize;
    uint64_t StructureTableAddress;
} __attribute__((__packed__)) FFSmbios30EntryPoint;

static_assert(offsetof(FFSmbios30EntryPoint, StructureTableAddress) == 0x10,
    "FFSmbios30EntryPoint: Wrong struct alignment");

typedef union FFSmbiosEntryPoint
{
    FFSmbios20EntryPoint Smbios20;
    FFSmbios30EntryPoint Smbios30;
} FFSmbiosEntryPoint;

const FFSmbiosHeaderTable* ffGetSmbiosHeaderTable()
{
    static FFstrbuf buffer;
    static FFSmbiosHeaderTable table;

    if (buffer.chars == NULL)
    {
        #ifdef __linux__
        if (!ffAppendFileBuffer("/sys/firmware/dmi/tables/DMI", &buffer))
        #endif
        {
            #ifndef __sun
            FF_STRBUF_AUTO_DESTROY strEntryAddress = ffStrbufCreate();
            #ifdef __FreeBSD__
            if (!ffSettingsGetFreeBSDKenv("hint.smbios.0.mem", &strEntryAddress))
                return NULL;
            #elif defined(__linux__)
            {
                FF_STRBUF_AUTO_DESTROY systab = ffStrbufCreate();
                if (!ffAppendFileBuffer("/sys/firmware/efi/systab", &systab))
                    return NULL;
                if (!ffParsePropLines(systab.chars, "SMBIOS3=", &strEntryAddress) &&
                    !ffParsePropLines(systab.chars, "SMBIOS=", &strEntryAddress))
                    return NULL;
            }
            #endif

            loff_t entryAddress = (loff_t) strtol(strEntryAddress.chars, NULL, 16);
            if (entryAddress == 0) return NULL;

            FF_AUTO_CLOSE_FD int fd = open("/dev/mem", O_RDONLY);
            if (fd < 0) return NULL;

            FFSmbiosEntryPoint entryPoint;
            if (pread(fd, &entryPoint, sizeof(entryPoint), entryAddress) < 0x10)
            {
                // `pread /dev/mem` returns EFAULT in FreeBSD
                // https://stackoverflow.com/questions/69372330/how-to-read-dev-mem-using-read
                void* p = mmap(NULL, sizeof(entryPoint), PROT_READ, MAP_SHARED, fd, entryAddress);
                if (p == MAP_FAILED) return NULL;
                memcpy(&entryPoint, p, sizeof(entryPoint));
                munmap(p, sizeof(entryPoint));
            }
            #else
            FF_AUTO_CLOSE_FD int fd = open("/dev/smbios", O_RDONLY);
            if (fd < 0) return NULL;

            FFSmbiosEntryPoint entryPoint;
            if (!ffReadFDData(fd, sizeof(entryPoint), &entryPoint)) return NULL;
            #endif

            uint32_t tableLength = 0;
            loff_t tableAddress = 0;
            if (memcmp(entryPoint.Smbios20.AnchorString, "_SM_", sizeof(entryPoint.Smbios20.AnchorString)) == 0)
            {
                if (entryPoint.Smbios20.EntryPointLength != sizeof(entryPoint.Smbios20))
                    return NULL;
                tableLength = entryPoint.Smbios20.StructureTableLength;
                tableAddress = (loff_t) entryPoint.Smbios20.StructureTableAddress;
            }
            else if (memcmp(entryPoint.Smbios30.AnchorString, "_SM3_", sizeof(entryPoint.Smbios30.AnchorString)) == 0)
            {
                if (entryPoint.Smbios30.EntryPointLength != sizeof(entryPoint.Smbios30))
                    return NULL;
                tableLength = entryPoint.Smbios30.StructureTableMaximumSize;
                tableAddress = (loff_t) entryPoint.Smbios30.StructureTableAddress;
            }

            ffStrbufEnsureFixedLengthFree(&buffer, tableLength);
            if (pread(fd, buffer.chars, tableLength, tableAddress) == tableLength)
            {
                buffer.length = tableLength;
                buffer.chars[buffer.length] = '\0';
            }
            else
            {
                // entryPoint.StructureTableAddress must be page aligned.
                // Unaligned physical memory access results in all kinds of crashes.
                void* p = mmap(NULL, tableLength, PROT_READ, MAP_SHARED, fd, tableAddress);
                if (p == MAP_FAILED)
                {
                    ffStrbufDestroy(&buffer); // free buffer and reset state
                    return NULL;
                }
                ffStrbufSetNS(&buffer, tableLength, (char*) p);
                munmap(p, tableLength);
            }
        }

        for (
            const FFSmbiosHeader* header = (const FFSmbiosHeader*) buffer.chars;
            (const uint8_t*) header < (const uint8_t*) buffer.chars + buffer.length;
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
#elif defined(_WIN32)
#include <windows.h>

#pragma GCC diagnostic ignored "-Wmultichar"

typedef struct FFRawSmbiosData
{
    uint8_t Used20CallingMethod;
    uint8_t SMBIOSMajorVersion;
    uint8_t SMBIOSMinorVersion;
    uint8_t DmiRevision;
    uint32_t Length;
    uint8_t SMBIOSTableData[];
} FFRawSmbiosData;

const FFSmbiosHeaderTable* ffGetSmbiosHeaderTable()
{
    static FFRawSmbiosData* buffer;
    static FFSmbiosHeaderTable table;

    if (!buffer)
    {
        const DWORD signature = 'RSMB';
        uint32_t bufSize = GetSystemFirmwareTable(signature, 0, NULL, 0);
        if (bufSize <= sizeof(FFRawSmbiosData))
            return NULL;

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
