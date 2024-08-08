#include "elf.h"

#ifdef FF_HAVE_ELF

#include "common/io/io.h"
#include "common/library.h"
#include "util/stringUtils.h"

#include <libelf.h>
#include <fcntl.h>

const char* ffElfExtractStrings(const char* elfFile, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata)
{
    FF_LIBRARY_LOAD(libelf, &instance.config.library.libelf, "dlopen libelf" FF_LIBRARY_EXTENSION " failed", "libelf" FF_LIBRARY_EXTENSION, 1);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libelf, elf_version)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libelf, elf_begin)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libelf, elf_getshdrstrndx)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libelf, elf_nextscn)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libelf, elf64_getshdr)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libelf, elf32_getshdr)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libelf, elf_getdata)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libelf, elf_strptr)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libelf, elf_end)

    if (ffelf_version(EV_CURRENT) == EV_NONE) return "elf_version() failed";

    FF_AUTO_CLOSE_FD int fd = open(elfFile, O_RDONLY, 0);
    if (fd < 0) return "open() failed";

    Elf* elf = ffelf_begin(fd, ELF_C_READ, NULL);
    if (elf == NULL) return "elf_begin() failed";

    size_t shstrndx = 0;
    if (ffelf_getshdrstrndx(elf, &shstrndx) < 0)
    {
        ffelf_end(elf);
        return "elf_getshdrstrndx() failed";
    }

    Elf_Scn* scn = NULL;
    while ((scn = ffelf_nextscn(elf, scn)) != NULL)
    {
        Elf64_Shdr* shdr64 = ffelf64_getshdr(scn);
        Elf32_Shdr* shdr32 = NULL;
        if (shdr64 == NULL)
        {
            shdr32 = ffelf32_getshdr(scn);
            if (shdr32 == NULL) continue;
        }

        const char* name = ffelf_strptr(elf, shstrndx, shdr64 ? shdr64->sh_name : shdr32->sh_name);
        if (name == NULL || !ffStrEquals(name, ".rodata")) continue;

        Elf_Data* data = ffelf_getdata(scn, NULL);
        if (data == NULL) continue;

        for (size_t off = 0; off < data->d_size; ++off)
        {
            const char* p = (const char*) data->d_buf + off;
            if (*p == '\0') continue;
            uint32_t len = (uint32_t) strlen(p);
            if (*p >= ' ' && *p <= '~') // Ignore control characters
            {
                if (!cb(p, len, userdata)) break;
            }
            off += len;
        }

        break;
    }

    ffelf_end(elf);
    return NULL;
}

#else

const char* ffElfExtractStrings(const char* elfFile, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata)
{
    FF_UNUSED(elfFile, cb, userdata);
    return "Fastfetch was built without libelf support";
}

#endif
