#include "binary.h"
#include "common/io/io.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

#include <windows.h>
#include <imagehlp.h>
#include <stdlib.h>
#include <string.h>

const char* ffBinaryExtractStrings(const char *peFile, bool (*cb)(const char *str, uint32_t len, void *userdata), void *userdata, uint32_t minLength)
{
    __attribute__((__cleanup__(UnMapAndLoad))) LOADED_IMAGE loadedImage = {};
    if (!MapAndLoad(peFile, NULL, &loadedImage, FALSE, TRUE))
        return "File could not be loaded";

    for (ULONG i = 0; i < loadedImage.NumberOfSections; ++i)
    {
        PIMAGE_SECTION_HEADER section = &loadedImage.Sections[i];
        if ((section->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA) && ffStrEquals((const char*) section->Name, ".rdata"))
        {
            uint8_t *data = (uint8_t *) loadedImage.MappedAddress + section->PointerToRawData;

            for (size_t off = 0; off < section->SizeOfRawData; ++off)
            {
                const char* p = (const char*) data + off;
                if (*p == '\0') continue;
                uint32_t len = (uint32_t) strlen(p);
                if (len < minLength) continue;
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
