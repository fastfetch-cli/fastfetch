#include "common/binary.h"
#include "common/io.h"
#include "common/stringUtils.h"
#include "common/windows/nt.h"

#include <windows.h>
#include <stdlib.h>
#include <string.h>

/**
 * Extracts string literals from a PE (Windows) executable
 *
 * This function maps the PE file into memory, locates the .rdata section
 * (which typically contains string literals), and scans it for valid strings.
 * Each string found is passed to the callback function for processing.
 */
const char* ffBinaryExtractStrings(const char *peFile, bool (*cb)(const char *str, uint32_t len, void *userdata), void *userdata, uint32_t minLength)
{
    FF_AUTO_CLOSE_FD HANDLE hFile = CreateFileA(peFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return "CreateFileA() failed";

    FF_AUTO_CLOSE_FD HANDLE hSection = NULL;
    if (!NT_SUCCESS(NtCreateSection(&hSection, SECTION_MAP_READ, NULL, NULL, PAGE_READONLY, SEC_COMMIT, hFile)))
        return "NtCreateSection() failed";

    PVOID base = NULL;
    SIZE_T viewSize = 0;
    if (!NT_SUCCESS(NtMapViewOfSection(hSection, NtCurrentProcess(), &base, 0, 0, NULL, &viewSize, ViewUnmap, 0, PAGE_READONLY)))
        return "NtMapViewOfSection() failed";

    PIMAGE_NT_HEADERS ntHeaders = RtlImageNtHeader(base);
    if (!ntHeaders)
    {
        NtUnmapViewOfSection(NtCurrentProcess(), base);
        return "RtlImageNtHeader() failed";
    }

    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);
    for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i, ++section)
    {
        // Look for initialized data sections with the name ".rdata" which typically contains string literals
        if ((section->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) && ffStrEquals((const char*) section->Name, ".rdata"))
        {
            uint8_t *data = (uint8_t *) base + section->PointerToRawData;

            // Scan the section for string literals
            for (size_t off = 0; off < section->SizeOfRawData; ++off)
            {
                const char* p = (const char*) data + off;
                if (*p == '\0') continue;
                uint32_t len = (uint32_t) strlen(p);
                if (len < minLength) continue;
                // Only process printable ASCII characters
                if (*p >= ' ' && *p <= '~') // Ignore control characters
                {
                    if (!cb(p, len, userdata)) break;
                }
                off += len;
            }
        }
    }

    NtUnmapViewOfSection(NtCurrentProcess(), base);
    return NULL;
}
