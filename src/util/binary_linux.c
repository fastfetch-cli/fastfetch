#include "binary.h"

#if defined(FF_HAVE_ELF) || defined(__sun) || (defined(__FreeBSD__) && !defined(__DragonFly__)) || defined(__OpenBSD__) || defined(__NetBSD__)

#include "common/io/io.h"
#include "common/library.h"
#include "util/stringUtils.h"

#include <libelf.h> // #1254
#include <fcntl.h>

/**
 * Structure to hold dynamically loaded libelf function pointers
 */
struct FFElfData {
    FF_LIBRARY_SYMBOL(elf_version)
    FF_LIBRARY_SYMBOL(elf_begin)
    FF_LIBRARY_SYMBOL(elf_getshdrstrndx)
    FF_LIBRARY_SYMBOL(elf_nextscn)
    FF_LIBRARY_SYMBOL(elf64_getshdr)
    FF_LIBRARY_SYMBOL(elf32_getshdr)
    FF_LIBRARY_SYMBOL(elf_getdata)
    FF_LIBRARY_SYMBOL(elf_strptr)
    FF_LIBRARY_SYMBOL(elf_end)

    bool inited;
} elfData;

/**
 * Extracts string literals from an ELF (Linux/Unix) binary file
 *
 * This function loads the libelf library dynamically, opens the ELF file,
 * locates the .rodata section (which contains string literals), and
 * scans it for valid strings. Each string found is passed to the
 * callback function for processing.
 *
 * The function supports both 32-bit and 64-bit ELF formats.
 */
const char* ffBinaryExtractStrings(const char* elfFile, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata, uint32_t minLength)
{
    // Initialize libelf if not already done
    if (!elfData.inited)
    {
        elfData.inited = true;
        FF_LIBRARY_LOAD(libelf, "dlopen libelf" FF_LIBRARY_EXTENSION " failed", "libelf" FF_LIBRARY_EXTENSION, 1);
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libelf, elfData, elf_version)
        if (elfData.ffelf_version(EV_CURRENT) == EV_NONE) return "elf_version() failed";

        // Load all required libelf functions
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libelf, elfData, elf_begin)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libelf, elfData, elf_getshdrstrndx)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libelf, elfData, elf_nextscn)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libelf, elfData, elf64_getshdr)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libelf, elfData, elf32_getshdr)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libelf, elfData, elf_getdata)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libelf, elfData, elf_strptr)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libelf, elfData, elf_end)

        libelf = NULL;
    }

    if (elfData.ffelf_end == NULL)
        return "load libelf failed";

    // Open the ELF file
    FF_AUTO_CLOSE_FD int fd = open(elfFile, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return "open() failed";

    Elf* elf = elfData.ffelf_begin(fd, ELF_C_READ, NULL);
    if (elf == NULL) return "elf_begin() failed";

    // Get the section header string table index
    size_t shstrndx = 0;
    if (elfData.ffelf_getshdrstrndx(elf, &shstrndx) < 0)
    {
        elfData.ffelf_end(elf);
        return "elf_getshdrstrndx() failed";
    }

    // Iterate through all sections, looking for .rodata which contains string literals
    Elf_Scn* scn = NULL;
    while ((scn = elfData.ffelf_nextscn(elf, scn)) != NULL)
    {
        // Try 64-bit section header first, then 32-bit if that fails
        Elf64_Shdr* shdr64 = elfData.ffelf64_getshdr(scn);
        Elf32_Shdr* shdr32 = NULL;
        if (shdr64 == NULL)
        {
            shdr32 = elfData.ffelf32_getshdr(scn);
            if (shdr32 == NULL) continue;
        }

        // Get the section name and check if it's .rodata
        const char* name = elfData.ffelf_strptr(elf, shstrndx, shdr64 ? shdr64->sh_name : shdr32->sh_name);
        if (name == NULL || !ffStrEquals(name, ".rodata")) continue;

        // Get the section data
        Elf_Data* data = elfData.ffelf_getdata(scn, NULL);
        if (data == NULL) continue;

        // Scan the section for string literals
        for (size_t off = 0; off < data->d_size; ++off)
        {
            const char* p = (const char*) data->d_buf + off;
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

        break;
    }

    elfData.ffelf_end(elf);
    return NULL;
}

#else

/**
 * Fallback implementation when libelf is not available
 */
const char* ffBinaryExtractStrings(const char* file, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata, uint32_t minLength)
{
    FF_UNUSED(file, cb, userdata, minLength);
    return "Fastfetch was built without libelf support";
}

#endif
