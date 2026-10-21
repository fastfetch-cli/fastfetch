#pragma once

#include "fastfetch.h"

// Locating an entry inside a zip, which is what both the jar reader and the app label reader need:
// one looks for `classesN.dex`, the other for the resource table of an APK.
//
// The entry is found through the central directory rather than by scanning for its local header,
// and that is not a micro-optimisation. A local header is preceded by everything that came before
// it, so scanning costs a pass over all of it, once per lookup: `framework.jar` is 52 MB and its
// `classes2.dex` starts 9.6 MB in. Measured on the test device, where the sound module resolves
// seven codes, looking the entries up by scanning was the difference between 30 ms and 2 seconds.
//
// The sizes and the compression method are read from the directory rather than from the entry's own
// local header, which is the more reliable of the two: a local header is allowed to leave its sizes
// at zero and put them in a data descriptor behind the payload, and an archive writer is free to
// choose that. Only the local header's two length fields are used, because they are what says where
// the payload begins -- the local extra field does not have to match the central one.

// The zip local file header, from APPNOTE.TXT 4.3.7.
#define FF_ZIP_LOCAL_HEADER_SIZE 30
#define FF_ZIP_OFF_NAME_LENGTH 26
#define FF_ZIP_OFF_EXTRA_LENGTH 28

// The central directory.
#define FF_ZIP_CENTRAL_MAGIC "PK\x01\x02"
#define FF_ZIP_CENTRAL_SIZE 46
#define FF_ZIP_OFF_CENTRAL_METHOD 10
#define FF_ZIP_OFF_CENTRAL_COMPRESSED_SIZE 20
#define FF_ZIP_OFF_CENTRAL_UNCOMPRESSED_SIZE 24
#define FF_ZIP_OFF_CENTRAL_NAME_LENGTH 28
#define FF_ZIP_OFF_CENTRAL_EXTRA_LENGTH 30
#define FF_ZIP_OFF_CENTRAL_COMMENT_LENGTH 32
#define FF_ZIP_OFF_CENTRAL_LOCAL_OFFSET 42

// The end of central directory record, which points at the directory. It is the last thing in the
// file, behind a comment of at most 64 KiB, so it is searched for backwards over that much.
#define FF_ZIP_EOCD_MAGIC "PK\x05\x06"
#define FF_ZIP_EOCD_SIZE 22
#define FF_ZIP_EOCD_MAX_COMMENT 0xffffu
#define FF_ZIP_OFF_EOCD_DIRECTORY_SIZE 12
#define FF_ZIP_OFF_EOCD_DIRECTORY 16

// A size or offset of 0xFFFFFFFF means the real one is in a zip64 extra field. AOSP's framework jars
// and the APKs measured so far are orders of magnitude below that, and carrying the extra field
// reader for them is not worth it.
#define FF_ZIP_ZIP64_SENTINEL 0xffffffffu

#define FF_ZIP_METHOD_STORED 0
#define FF_ZIP_METHOD_DEFLATED 8

// One entry, as the archive describes it. `payload` points into the mapping the caller passed in and
// is not a copy: for a STORED entry it is the file's own bytes, and for a DEFLATED one it is the raw
// deflate stream, which the caller inflates with `uncompressedSize` as the bound.
typedef struct FFAndroidZipEntry {
    const uint8_t* payload;
    size_t payloadSize;
    uint32_t uncompressedSize;
    uint16_t method;
} FFAndroidZipEntry;

// Resolves `entry` through the archive's central directory, writing where its payload is into
// `result`. Returns nullptr on success, a static message otherwise -- an archive that is not a zip,
// a directory that is out of range, an entry that is not there and an entry stored as zip64 are all
// reported rather than answered with a pointer into nothing.
[[gnu::nonnull(1, 3, 4), nodiscard]] const char* ffAndroidZipFindEntry(const uint8_t* archive, size_t archiveSize, const char* entry, FFAndroidZipEntry* result);
