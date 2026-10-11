#include "common/android/zip.h"
#include "common/debug.h"

#include <string.h>

static uint16_t zipU16(const uint8_t* p) {
    return (uint16_t) ((uint16_t) p[0] | (uint16_t) ((uint16_t) p[1] << 8));
}

static uint32_t zipU32(const uint8_t* p) {
    return (uint32_t) p[0] | ((uint32_t) p[1] << 8) | ((uint32_t) p[2] << 16) | ((uint32_t) p[3] << 24);
}

const char* ffAndroidZipFindEntry(const uint8_t* archive, size_t archiveSize, const char* entry, FFAndroidZipEntry* result) {
    const size_t entryLength = strlen(entry);
    if (archiveSize < FF_ZIP_EOCD_SIZE) {
        return "The archive is too small to be a zip";
    }

    // The record is the last thing in the file, so it is searched for backwards -- over at most the
    // 64 KiB a trailing comment can take, not over the whole archive.
    const size_t newest = archiveSize - FF_ZIP_EOCD_SIZE;
    const size_t oldest = archiveSize > FF_ZIP_EOCD_MAX_COMMENT + FF_ZIP_EOCD_SIZE
        ? archiveSize - FF_ZIP_EOCD_MAX_COMMENT - FF_ZIP_EOCD_SIZE
        : 0;
    size_t eocd = SIZE_MAX;
    for (size_t i = newest;; --i) {
        if (memcmp(archive + i, FF_ZIP_EOCD_MAGIC, sizeof(FF_ZIP_EOCD_MAGIC) - 1) == 0) {
            eocd = i;
            break;
        }
        if (i == oldest) {
            // The walk stops here rather than at zero: a `size_t` countdown that has to reach zero
            // before it stops would wrap and read the whole file backwards, which is the very scan
            // this is here to avoid.
            break;
        }
    }
    if (eocd == SIZE_MAX) {
        FF_DEBUG("No end-of-central-directory record in the last %zu bytes of a %zu byte archive",
            newest - oldest + FF_ZIP_EOCD_SIZE, archiveSize);
        return "The archive has no zip directory";
    }

    const uint32_t directorySize = zipU32(archive + eocd + FF_ZIP_OFF_EOCD_DIRECTORY_SIZE);
    const uint32_t directoryOffset = zipU32(archive + eocd + FF_ZIP_OFF_EOCD_DIRECTORY);
    if (directoryOffset > archiveSize || directorySize > archiveSize - directoryOffset) {
        return "The zip directory is out of range";
    }

    // The directory is walked to its end rather than for the entry count the record carries: that
    // count is a 16-bit field which a 65535-entry archive pins at 0xFFFF and moves into a zip64
    // record, while the two bounds above describe the directory itself on every archive this small.
    const size_t directoryEnd = (size_t) directoryOffset + directorySize;
    for (size_t offset = directoryOffset; offset + FF_ZIP_CENTRAL_SIZE <= directoryEnd;) {
        if (memcmp(archive + offset, FF_ZIP_CENTRAL_MAGIC, sizeof(FF_ZIP_CENTRAL_MAGIC) - 1) != 0) {
            return "A zip directory entry is malformed";
        }
        const uint16_t nameLength = zipU16(archive + offset + FF_ZIP_OFF_CENTRAL_NAME_LENGTH);
        const uint16_t extraLength = zipU16(archive + offset + FF_ZIP_OFF_CENTRAL_EXTRA_LENGTH);
        const uint16_t commentLength = zipU16(archive + offset + FF_ZIP_OFF_CENTRAL_COMMENT_LENGTH);
        const size_t entrySize = FF_ZIP_CENTRAL_SIZE + (size_t) nameLength + (size_t) extraLength + (size_t) commentLength;
        if (entrySize > directoryEnd - offset) {
            FF_DEBUG("A zip entry claims %zu bytes but only %zu remain in the directory",
                entrySize, (size_t) (directoryEnd - offset));
            return "A zip directory entry runs past the directory";
        }

        if (nameLength == entryLength && memcmp(archive + offset + FF_ZIP_CENTRAL_SIZE, entry, entryLength) == 0) {
            const uint32_t compressedSize = zipU32(archive + offset + FF_ZIP_OFF_CENTRAL_COMPRESSED_SIZE);
            const uint32_t uncompressedSize = zipU32(archive + offset + FF_ZIP_OFF_CENTRAL_UNCOMPRESSED_SIZE);
            const uint32_t localOffset = zipU32(archive + offset + FF_ZIP_OFF_CENTRAL_LOCAL_OFFSET);
            if (compressedSize == FF_ZIP_ZIP64_SENTINEL || uncompressedSize == FF_ZIP_ZIP64_SENTINEL || localOffset == FF_ZIP_ZIP64_SENTINEL) {
                FF_DEBUG("The zip entry \"%s\" is zip64: compressed=%u, uncompressed=%u, localOffset=%u",
                    entry, compressedSize, uncompressedSize, localOffset);
                return "The zip entry is stored as zip64, which is not supported";
            }
            if (localOffset > archiveSize || FF_ZIP_LOCAL_HEADER_SIZE > archiveSize - localOffset) {
                return "A zip local header is out of range";
            }

            const uint16_t localNameLength = zipU16(archive + localOffset + FF_ZIP_OFF_NAME_LENGTH);
            const uint16_t localExtraLength = zipU16(archive + localOffset + FF_ZIP_OFF_EXTRA_LENGTH);
            const size_t headerSize = FF_ZIP_LOCAL_HEADER_SIZE + (size_t) localNameLength + (size_t) localExtraLength;
            if (headerSize > archiveSize - localOffset || compressedSize > archiveSize - localOffset - headerSize) {
                return "The zip entry bounds are out of range";
            }

            result->method = zipU16(archive + offset + FF_ZIP_OFF_CENTRAL_METHOD);
            result->uncompressedSize = uncompressedSize;
            result->payload = archive + localOffset + headerSize;
            result->payloadSize = compressedSize;
            return nullptr;
        }

        offset += entrySize;
    }
    FF_DEBUG("No entry in the %zu byte archive is named \"%s\"", archiveSize, entry);
    return "The archive has no entry with that name";
}
