#include "binary.h"
#include "common/io/io.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <mach-o/loader.h>
#include <mach-o/swap.h>
#include <mach-o/fat.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // swap_fat_arch

// Ref: https://github.com/AlexDenisov/segment_dumper/blob/master/main.c

/**
 * Helper function to read data from a file at a specific offset
 */
static inline bool readData(FILE *objFile, void *buf, size_t size, off_t offset)
{
    fseek(objFile, offset, SEEK_SET);
    return fread(buf, 1, size, objFile) == size;
}

/**
 * Handles a Mach-O section by extracting strings from the __cstring section
 *
 * @param objFile File handle to the Mach-O object file
 * @param name Section name to check
 * @param offset Offset of the section in the file
 * @param size Size of the section
 * @param cb Callback function to process strings
 * @param userdata User data for the callback
 * @param minLength Minimum string length to extract
 *
 * @return true to continue processing, false to stop
 */
static bool handleMachSection(FILE *objFile, const char *name, off_t offset, size_t size, bool (*cb)(const char *str, uint32_t len, void *userdata), void *userdata, uint32_t minLength)
{
    if (!ffStrEquals(name, "__cstring")) return true;

    FF_AUTO_FREE char* data = (char*) malloc(size);
    if (!readData(objFile, data, size, offset))
        return true;

    for (size_t off = 0; off < size; ++off)
    {
        const char* p = (const char*) data + off;
        if (*p == '\0') continue;
        uint32_t len = (uint32_t) strlen(p);
        if (len < minLength) continue;
        if (*p >= ' ' && *p <= '~') // Ignore control characters
        {
            if (!cb(p, len, userdata)) return false;
        }
        off += len;
    }
    return true;
}

/**
 * Processes a Mach-O header (32-bit or 64-bit)
 *
 * This function parses the load commands in a Mach-O header, looking for
 * LC_SEGMENT or LC_SEGMENT_64 commands that contain the __TEXT segment.
 * It then processes the sections within that segment to extract strings.
 *
 * @param objFile File handle to the Mach-O object file
 * @param offset Offset of the Mach header in the file
 * @param is_64 Whether this is a 64-bit Mach-O header
 * @param cb Callback function to process strings
 * @param userdata User data for the callback
 * @param minLength Minimum string length to extract
 *
 * @return NULL on success, error message on failure
 */
static const char* dumpMachHeader(FILE *objFile, off_t offset, bool is_64, bool (*cb)(const char *str, uint32_t len, void *userdata), void *userdata, uint32_t minLength)
{
    uint32_t ncmds;
    off_t loadCommandsOffset = offset;

    if (is_64)
    {
        struct mach_header_64 header;
        if (!readData(objFile, &header, sizeof(header), offset))
            return "read mach header failed";

        ncmds = header.ncmds;
        loadCommandsOffset += sizeof(header);
    }
    else
    {
        struct mach_header header;
        if (!readData(objFile, &header, sizeof(header), offset))
            return "read mach header failed";

        ncmds = header.ncmds;
        loadCommandsOffset += sizeof(header);
    }

    off_t commandOffset = loadCommandsOffset;
    struct load_command cmd = {};
    for (uint32_t i = 0U; i < ncmds; i++, commandOffset += cmd.cmdsize)
    {
        if (!readData(objFile, &cmd, sizeof(cmd), commandOffset))
            continue;

        if (cmd.cmd == LC_SEGMENT_64)
        {
            struct segment_command_64 segment;
            if (!readData(objFile, &segment, sizeof(segment), commandOffset))
                continue;

            if (!ffStrEquals(segment.segname, "__TEXT")) continue;

            for (uint32_t j = 0U; j < segment.nsects; j++)
            {
                struct section_64 section;
                if (!readData(objFile, &section, sizeof(section), (off_t) ((size_t) commandOffset + sizeof(segment) + j * sizeof(section))))
                    continue;

                if (!handleMachSection(objFile, section.sectname, section.offset, section.size, cb, userdata, minLength))
                    return NULL;
            }
        }
        else if (cmd.cmd == LC_SEGMENT)
        {
            struct segment_command segment;
            if (!readData(objFile, &segment, sizeof(segment), commandOffset))
                continue;

            if (!ffStrEquals(segment.segname, "__TEXT")) continue;

            for (uint32_t j = 0; j < segment.nsects; j++)
            {
                struct section section;
                if (!readData(objFile, &section, sizeof(section), (off_t) ((size_t) commandOffset + sizeof(segment) + j * sizeof(section))))
                    continue;

                if (!handleMachSection(objFile, section.sectname, section.offset, section.size, cb, userdata, minLength))
                    return NULL;
            }
        }

        return NULL;
    }

    return NULL;
}

/**
 * Processes a Fat binary header (Universal binary)
 *
 * This function handles the fat header of a universal binary, which can contain
 * multiple Mach-O binaries for different architectures. It extracts and processes
 * each embedded Mach-O file.
 *
 * @param objFile File handle to the universal binary
 * @param cb Callback function to process strings
 * @param userdata User data for the callback
 * @param minLength Minimum string length to extract
 *
 * @return NULL on success, error message on failure
 */
static const char* dumpFatHeader(FILE *objFile, bool (*cb)(const char *str, uint32_t len, void *userdata), void *userdata, uint32_t minLength)
{
    struct fat_header header;
    if (!readData(objFile, &header, sizeof(header), 0))
        return "read fat header failed";

    bool needSwap = header.magic == FAT_CIGAM || header.magic == FAT_CIGAM_64;

    if (needSwap) swap_fat_header(&header, NX_UnknownByteOrder);

    for (uint32_t i = 0U; i < header.nfat_arch; i++)
    {
        off_t machHeaderOffset = 0;
        if (header.magic == FAT_MAGIC)
        {
            struct fat_arch arch;
            if (!readData(objFile, &arch, sizeof(arch), (off_t) (sizeof(header) + i * sizeof(arch))))
                continue;

            if (needSwap)
                swap_fat_arch(&arch, 1, NX_UnknownByteOrder);
            machHeaderOffset = (off_t)arch.offset;
        }
        else
        {
            struct fat_arch_64 arch;
            if (!readData(objFile, &arch, sizeof(arch), (off_t) (sizeof(header) + i * sizeof(arch))))
                continue;

            if (needSwap)
                swap_fat_arch_64(&arch, 1, NX_UnknownByteOrder);

            machHeaderOffset = (off_t)arch.offset;
        }

        uint32_t magic;
        if (!readData(objFile, &magic, sizeof(magic), machHeaderOffset))
            continue;

        if (magic == MH_MAGIC_64 || magic == MH_MAGIC)
        {
            dumpMachHeader(objFile, machHeaderOffset, magic == MH_MAGIC_64, cb, userdata, minLength);
            return NULL;
        }
    }
    return "Unsupported fat header";
}

/**
 * Extracts string literals from a Mach-O (Apple) binary file
 *
 * This function supports both single-architecture Mach-O files and
 * universal binaries (fat binaries) containing multiple architectures.
 * It locates the __cstring section in the __TEXT segment which contains
 * the string literals used in the program.
 */
const char *ffBinaryExtractStrings(const char *machoFile, bool (*cb)(const char *str, uint32_t len, void *userdata), void *userdata, uint32_t minLength)
{
    FF_AUTO_CLOSE_FILE FILE *objFile = fopen(machoFile, "rb");
    if (objFile == NULL)
        return "File could not be opened";

    // Read the magic number to determine the type of binary
    uint32_t magic;
    if (!readData(objFile, &magic, sizeof(magic), 0))
        return "read magic number failed";

    // Check for supported formats
    // MH_CIGAM and MH_CIGAM_64 seem to be no longer used, as `swap_mach_header` is marked as deprecated.
    // However FAT_CIGAM and FAT_CIGAM_64 are still used (/usr/bin/vim).
    if (magic != MH_MAGIC && magic != MH_MAGIC_64 && magic != FAT_CIGAM && magic != FAT_CIGAM_64 && magic != FAT_MAGIC && magic != FAT_MAGIC_64)
        return "Unsupported format or big endian mach-o file";

    // Process either a fat binary or a regular Mach-O binary
    if (magic == FAT_MAGIC || magic == FAT_MAGIC_64 || magic == FAT_CIGAM || magic == FAT_CIGAM_64)
        return dumpFatHeader(objFile, cb, userdata, minLength);
    else
        return dumpMachHeader(objFile, 0, magic == MH_MAGIC_64, cb, userdata, minLength);
}
