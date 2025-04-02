#include "binary.h"
#include "common/io/io.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

#include <windows.h>
#include <imagehlp.h>
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
    // Use MapAndLoad with cleanup attribute to ensure proper unloading
    __attribute__((__cleanup__(UnMapAndLoad))) LOADED_IMAGE loadedImage = {};
    if (!MapAndLoad(peFile, NULL, &loadedImage, FALSE, TRUE))
        return "File could not be loaded";

    // Iterate through all sections in the PE file
    for (ULONG i = 0; i < loadedImage.NumberOfSections; ++i)
    {
        PIMAGE_SECTION_HEADER section = &loadedImage.Sections[i];
        // Look for initialized data sections with the name ".rdata" which typically contains string literals
        if ((section->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) && ffStrEquals((const char*) section->Name, ".rdata"))
        {
            uint8_t *data = (uint8_t *) loadedImage.MappedAddress + section->PointerToRawData;

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

    return NULL;
}
