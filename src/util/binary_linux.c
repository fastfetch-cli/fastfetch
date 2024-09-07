#include "binary.h"

#if defined(FF_HAVE_ELF) || defined(__sun) || defined(__FreeBSD__)

#include "common/io/io.h"
#include "common/library.h"
#include "util/stringUtils.h"

#include <libelf.h> // #1254
#include <fcntl.h>

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

const char* ffBinaryExtractStrings(const char* elfFile, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata, uint32_t minLength)
{
    if (!elfData.inited)
    {
        elfData.inited = true;
        FF_LIBRARY_LOAD(libelf, "dlopen libelf" FF_LIBRARY_EXTENSION " failed", "libelf" FF_LIBRARY_EXTENSION, 1);
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libelf, elfData, elf_version)
        if (elfData.ffelf_version(EV_CURRENT) == EV_NONE) return "elf_version() failed";

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

    FF_AUTO_CLOSE_FD int fd = open(elfFile, O_RDONLY, 0);
    if (fd < 0) return "open() failed";

    Elf* elf = elfData.ffelf_begin(fd, ELF_C_READ, NULL);
    if (elf == NULL) return "elf_begin() failed";

    size_t shstrndx = 0;
    if (elfData.ffelf_getshdrstrndx(elf, &shstrndx) < 0)
    {
        elfData.ffelf_end(elf);
        return "elf_getshdrstrndx() failed";
    }

    Elf_Scn* scn = NULL;
    while ((scn = elfData.ffelf_nextscn(elf, scn)) != NULL)
    {
        Elf64_Shdr* shdr64 = elfData.ffelf64_getshdr(scn);
        Elf32_Shdr* shdr32 = NULL;
        if (shdr64 == NULL)
        {
            shdr32 = elfData.ffelf32_getshdr(scn);
            if (shdr32 == NULL) continue;
        }

        const char* name = elfData.ffelf_strptr(elf, shstrndx, shdr64 ? shdr64->sh_name : shdr32->sh_name);
        if (name == NULL || !ffStrEquals(name, ".rodata")) continue;

        Elf_Data* data = elfData.ffelf_getdata(scn, NULL);
        if (data == NULL) continue;

        for (size_t off = 0; off < data->d_size; ++off)
        {
            const char* p = (const char*) data->d_buf + off;
            if (*p == '\0') continue;
            uint32_t len = (uint32_t) strlen(p);
            if (len < minLength) continue;
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

const char* ffBinaryExtractStrings(const char* file, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata, uint32_t minLength)
{
    FF_UNUSED(file, cb, userdata, minLength);
    return "Fastfetch was built without libelf support";
}

#endif
