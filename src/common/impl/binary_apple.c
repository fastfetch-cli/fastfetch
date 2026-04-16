#include "common/binary.h"
#include "common/io.h"
#include "common/stringUtils.h"

#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <mach-o/loader.h>
#include <mach-o/swap.h>
#include <mach-o/fat.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // swap_fat_arch

// Ref: https://github.com/AlexDenisov/segment_dumper/blob/master/main.c

typedef struct {
    const uint8_t* data;
    size_t length;
} FFMemoryMapping;

static inline void wrapMunmap(FFMemoryMapping* mapping) {
    assert(mapping);
    if (mapping->data == NULL || mapping->data == MAP_FAILED)
        return;
    munmap((void*) mapping->data, mapping->length);
}

/**
 * Helper function to access data from a memory-mapped file at a specific offset
 */
static inline const void* readData(const FFMemoryMapping* mapping, size_t size, off_t offset) {
    if (offset < 0) {
        return NULL;
    }

    size_t start = (size_t) offset;
    if (start > mapping->length || size > mapping->length - start) {
        return NULL;
    }

    return mapping->data + start;
}

/**
 * Handles a Mach-O section by extracting strings from the __cstring section
 *
 * @param mapping Memory mapping of the Mach-O object file
 * @param name Section name to check
 * @param offset Offset of the section in the file
 * @param size Size of the section
 * @param cb Callback function to process strings
 * @param userdata User data for the callback
 * @param minLength Minimum string length to extract
 *
 * @return true to continue processing, false to stop
 */
static bool handleMachSection(const FFMemoryMapping* mapping, const char* name, off_t offset, size_t size, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata, uint32_t minLength) {
    if (!ffStrEquals(name, "__cstring")) {
        return true;
    }

    const char* data = readData(mapping, size, offset);
    if (!data) {
        return true;
    }

    for (size_t off = 0; off < size; ++off) {
        const char* p = data + off;
        if (*p == '\0') {
            continue;
        }
        uint32_t len = (uint32_t) strnlen(p, size - off);
        if (len < minLength) {
            continue;
        }
        if (*p >= ' ' && *p <= '~') { // Ignore control characters
            if (!cb(p, len, userdata)) {
                return false;
            }
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
 * @param mapping Memory mapping of the Mach-O object file
 * @param offset Offset of the Mach header in the file
 * @param is_64 Whether this is a 64-bit Mach-O header
 * @param cb Callback function to process strings
 * @param userdata User data for the callback
 * @param minLength Minimum string length to extract
 *
 * @return NULL on success, error message on failure
 */
static const char* dumpMachHeader(const FFMemoryMapping* mapping, off_t offset, bool is_64, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata, uint32_t minLength) {
    uint32_t ncmds;
    off_t loadCommandsOffset = offset;

    if (is_64) {
        const struct mach_header_64* header = readData(mapping, sizeof(struct mach_header_64), offset);
        if (!header) {
            return "read mach header failed";
        }

        ncmds = header->ncmds;
        loadCommandsOffset += sizeof(*header);
    } else {
        const struct mach_header* header = readData(mapping, sizeof(struct mach_header), offset);
        if (!header) {
            return "read mach header failed";
        }

        ncmds = header->ncmds;
        loadCommandsOffset += sizeof(*header);
    }

    off_t commandOffset = loadCommandsOffset;
    const struct load_command* cmd = NULL;
    for (uint32_t i = 0U; i < ncmds; i++, commandOffset += cmd->cmdsize) {
        cmd = readData(mapping, sizeof(*cmd), commandOffset);
        if (!cmd) {
            break;
        }

        if (cmd->cmdsize < sizeof(*cmd)) {
            break;
        }

        if (cmd->cmd == LC_SEGMENT_64) {
            const struct segment_command_64* segment = readData(mapping, sizeof(struct segment_command_64), commandOffset);
            if (!segment) {
                continue;
            }

            if (!ffStrEquals(segment->segname, "__TEXT")) {
                continue;
            }

            for (uint32_t j = 0U; j < segment->nsects; j++) {
                off_t sectionOffset = commandOffset + (off_t) sizeof(*segment) + (off_t) (j * sizeof(struct section_64));
                const struct section_64* section = readData(mapping, sizeof(struct section_64), sectionOffset);
                if (!section) {
                    continue;
                }

                if (!handleMachSection(mapping, section->sectname, (off_t) section->offset, (size_t) section->size, cb, userdata, minLength)) {
                    return NULL;
                }
            }
        } else if (cmd->cmd == LC_SEGMENT) {
            const struct segment_command* segment = readData(mapping, sizeof(struct segment_command), commandOffset);
            if (!segment) {
                continue;
            }

            if (!ffStrEquals(segment->segname, "__TEXT")) {
                continue;
            }

            for (uint32_t j = 0; j < segment->nsects; j++) {
                off_t sectionOffset = commandOffset + (off_t) sizeof(*segment) + (off_t) (j * sizeof(struct section));
                const struct section* section = readData(mapping, sizeof(struct section), sectionOffset);
                if (!section) {
                    continue;
                }

                if (!handleMachSection(mapping, section->sectname, (off_t) section->offset, (size_t) section->size, cb, userdata, minLength)) {
                    return NULL;
                }
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
 * @param mapping Memory mapping of the universal binary
 * @param cb Callback function to process strings
 * @param userdata User data for the callback
 * @param minLength Minimum string length to extract
 *
 * @return NULL on success, error message on failure
 */
static const char* dumpFatHeader(const FFMemoryMapping* mapping, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata, uint32_t minLength) {
    const struct fat_header* headerRaw = readData(mapping, sizeof(struct fat_header), 0);
    if (!headerRaw) {
        return "read fat header failed";
    }

    struct fat_header header = *headerRaw;

    bool needSwap = header.magic == FAT_CIGAM || header.magic == FAT_CIGAM_64;

    if (needSwap) {
        swap_fat_header(&header, NX_UnknownByteOrder);
    }

    for (uint32_t i = 0U; i < header.nfat_arch; i++) {
        off_t machHeaderOffset = 0;
        if (header.magic == FAT_MAGIC) {
            off_t archOffset = (off_t) sizeof(struct fat_header) + (off_t) (i * sizeof(struct fat_arch));
            const struct fat_arch* archRaw = readData(mapping, sizeof(struct fat_arch), archOffset);
            if (!archRaw) {
                continue;
            }

            struct fat_arch arch = *archRaw;

            if (needSwap) {
                swap_fat_arch(&arch, 1, NX_UnknownByteOrder);
            }
            machHeaderOffset = (off_t) arch.offset;
        } else {
            off_t archOffset = (off_t) sizeof(struct fat_header) + (off_t) (i * sizeof(struct fat_arch_64));
            const struct fat_arch_64* archRaw = readData(mapping, sizeof(struct fat_arch_64), archOffset);
            if (!archRaw) {
                continue;
            }

            struct fat_arch_64 arch = *archRaw;

            if (needSwap) {
                swap_fat_arch_64(&arch, 1, NX_UnknownByteOrder);
            }

            machHeaderOffset = (off_t) arch.offset;
        }

        const uint32_t* magic = readData(mapping, sizeof(uint32_t), machHeaderOffset);
        if (!magic) {
            continue;
        }

        if (*magic == MH_MAGIC_64 || *magic == MH_MAGIC) {
            return dumpMachHeader(mapping, machHeaderOffset, *magic == MH_MAGIC_64, cb, userdata, minLength);
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
const char* ffBinaryExtractStrings(const char* machoFile, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata, uint32_t minLength) {
    FF_AUTO_CLOSE_FD int fd = open(machoFile, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return "File could not be opened";
    }

    struct stat st;
    if (fstat(fd, &st) != 0 || st.st_size <= 0) {
        return "Failed to stat file";
    }

    FF_A_CLEANUP(wrapMunmap) FFMemoryMapping mapping = {
        .data = mmap(NULL, (size_t) st.st_size, PROT_READ, MAP_PRIVATE, fd, 0),
        .length = (size_t) st.st_size,
    };
    if (mapping.data == MAP_FAILED) {
        return "mmap failed";
    }

    // Read the magic number to determine the type of binary
    const uint32_t* magic = readData(&mapping, sizeof(uint32_t), 0);
    if (!magic) {
        return "read magic number failed";
    }

    // Check for supported formats
    // MH_CIGAM and MH_CIGAM_64 seem to be no longer used, as `swap_mach_header` is marked as deprecated.
    // However FAT_CIGAM and FAT_CIGAM_64 are still used (/usr/bin/vim).
    if (*magic != MH_MAGIC && *magic != MH_MAGIC_64 && *magic != FAT_CIGAM && *magic != FAT_CIGAM_64 && *magic != FAT_MAGIC && *magic != FAT_MAGIC_64) {
        return "Unsupported format or big endian mach-o file";
    }

    // Process either a fat binary or a regular Mach-O binary
    if (*magic == FAT_MAGIC || *magic == FAT_MAGIC_64 || *magic == FAT_CIGAM || *magic == FAT_CIGAM_64) {
        return dumpFatHeader(&mapping, cb, userdata, minLength);
    } else {
        return dumpMachHeader(&mapping, 0, *magic == MH_MAGIC_64, cb, userdata, minLength);
    }
}
