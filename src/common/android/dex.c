#include "common/android/dex.h"
#include "common/debug.h"
#include "common/io.h"
#include "common/mallocHelper.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#ifdef FF_HAVE_ZLIB
    #include "common/library.h"
    #include <zlib.h>
#endif

// Offsets into the dex header, from https://source.android.com/docs/core/runtime/dex-format.
#define FF_DEX_ENDIAN_TAG 0x12345678u
#define FF_DEX_HEADER_SIZE 0x70
#define FF_DEX_OFF_FILE_SIZE 0x20
#define FF_DEX_OFF_ENDIAN_TAG 0x28
#define FF_DEX_OFF_STRING_IDS 0x3C
#define FF_DEX_OFF_TYPE_IDS_SIZE 0x40
#define FF_DEX_OFF_TYPE_IDS 0x44
#define FF_DEX_OFF_FIELD_IDS 0x54
#define FF_DEX_OFF_CLASS_DEFS_SIZE 0x60
#define FF_DEX_OFF_CLASS_DEFS 0x64

// class_def_item: [u32 class_idx][u32 access_flags][u32 superclass_idx][u32 interfaces_off]
// [u32 source_file_idx][u32 annotations_off][u32 class_data_off][u32 static_values_off]
#define FF_DEX_CLASS_DEF_SIZE 32
#define FF_DEX_OFF_CLASS_DEF_DATA 24
#define FF_DEX_OFF_CLASS_DEF_STATIC_VALUES 28

// field_id_item: [u16 class_idx][u16 type_idx][u32 name_idx]
#define FF_DEX_FIELD_ID_SIZE 8
#define FF_DEX_OFF_FIELD_ID_NAME 4

// The zip local file header, from APPNOTE.TXT 4.3.7. Only the two length fields are read from it:
// the payload starts right behind them, and the local extra field is allowed to differ from the
// central one, so the offset cannot be computed from the directory alone.
#define FF_ZIP_LOCAL_HEADER_SIZE 30
#define FF_ZIP_OFF_NAME_LENGTH 26
#define FF_ZIP_OFF_EXTRA_LENGTH 28

// The central directory. This is what makes an entry reachable without walking everything in front
// of it, and that matters here: framework.jar is 52 MB and its classes2.dex starts 9.6 MB in, so
// looking for a local header by scanning the file costs a pass over everything before it -- once per
// lookup. Measured on the test device, where the sound module resolves seven codes, that was the
// difference between 30 ms and 2 seconds.
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
// are orders of magnitude below that, and carrying the extra field reader for them is not worth it.
#define FF_ZIP_ZIP64_SENTINEL 0xffffffffu

#define FF_ZIP_METHOD_STORED 0
#define FF_ZIP_METHOD_DEFLATED 8

#define FF_DEX_ENTRY "classes.dex"
// How many `classesN.dex` entries a jar is walked for. A jar that spreads its classes over more
// than this still resolves everything in the entries that were walked; only a class that lives in
// one of the ones past the bound is reported as missing.
#define FF_DEX_MAX_ENTRIES 10
#define FF_DEX_MAGIC "dex\n"

static uint16_t dexU16(const uint8_t* p) {
    return (uint16_t) ((uint16_t) p[0] | (uint16_t) ((uint16_t) p[1] << 8));
}

static uint32_t dexU32(const uint8_t* p) {
    return (uint32_t) p[0] | ((uint32_t) p[1] << 8) | ((uint32_t) p[2] << 16) | ((uint32_t) p[3] << 24);
}

// The dex header points at its tables with offsets that a corrupt or hostile file is free to set
// anywhere, including past the end of the mapping. Testing `end - p` alone does not catch that: once
// `p` is past `end` the subtraction is negative and casting it to size_t turns it into a huge value,
// so the comparison passes and the read goes off the end. Both directions have to be one test.
static bool dexInRange(const uint8_t* dex, const uint8_t* end, const uint8_t* p, size_t size) {
    return p >= dex && p <= end && (size_t) (end - p) >= size;
}

// Same, for a table of `count` elements of `elementSize` bytes. `count` is 64-bit because the
// callers add one to an index that came out of the file, and doing that in 32 bits can wrap to zero
// and make the check pass for an index that is not in the table. Dividing rather than multiplying is
// what keeps a hostile `count` from wrapping the size_t on a 32-bit build -- armeabi-v7a is one, and
// there `count * 4` overflows for any count above 0x3FFFFFFF and the check passes again.
static bool dexTableInRange(const uint8_t* dex, const uint8_t* end, const uint8_t* p, uint64_t count, size_t elementSize) {
    if (p < dex || p > end) {
        return false;
    }
    return (size_t) (end - p) / elementSize >= count;
}

// LEB128, at most five bytes for the 32-bit values a dex stores this way. `end` bounds every read:
// the lengths and indexes decoded here come out of the file, so a truncated one must not walk off
// the end. A read that runs into `end` yields what was decoded so far, and the callers treat the
// resulting value as untrusted -- it is either compared against another count or used as an index
// that is range checked in turn.
static uint32_t dexUleb128(const uint8_t** p, const uint8_t* end) {
    uint32_t value = 0;
    for (int shift = 0; shift <= 28; shift += 7) {
        if (*p >= end) {
            break;
        }
        const uint8_t byte = *(*p)++;
        value |= (uint32_t) (byte & 0x7f) << shift;
        if ((byte & 0x80) == 0) {
            break;
        }
    }
    return value;
}

// The type tag a dex encoded_value carries in the low five bits of its header byte, per the
// `encoded_value` table of the dex format.
//
// VALUE_NULL and VALUE_BOOLEAN are the trap. Their value *is* the header -- the type for a null,
// the size nibble for a boolean -- and they carry **no payload bytes at all**, so the
// `((size - 1) << 5) | type` rule that describes every other tag does not describe them. Taking the
// size nibble at face value and reading one payload byte walks one byte too far, which pairs every
// field after that with the wrong value. It is not a hypothetical: `IAudioService$Stub` declares ten
// `PERMISSIONS_*` arrays that are all null, right behind its `DESCRIPTOR` string and ahead of its
// 324 transaction constants, and those ten spurious bytes made
// `TRANSACTION_getDevicesForAttributesUnprotected` read as 210 instead of 167 -- a code that reaches
// a different method. `IWifiManager$Stub` has none ahead of the field the wifi module reads, which
// is the only reason this went unnoticed.
//
// Naming VALUE_BOOLEAN also matters for a second reason: it is the tag a `static final boolean`
// carries, and treating it as VALUE_INT would hand back a boolean where an int was asked for.
typedef enum FFDexValueType : uint32_t {
    FF_DEX_VALUE_INT = 0x04,     // sign-extended; one to four payload bytes
    FF_DEX_VALUE_NULL = 0x1e,    // no payload
    FF_DEX_VALUE_BOOLEAN = 0x1f, // no payload; the boolean is in the header's size nibble
} FFDexValueType;

// encoded_value: one header byte holding ((size - 1) << 5) | type, then that many payload bytes.
//
// The `static_values` array is not a list of ints: `IInterface.DESCRIPTOR` is a `static final
// String` and sits in it, and a real Stub class mixes ints, strings, nulls and booleans. So a value
// whose tag is not VALUE_INT is skipped -- its payload is stepped over to keep the walk aligned with
// the field list -- and only `isInt` reports whether the *wanted* field ended up being one. Failing
// on the first non-int instead would abort on `DESCRIPTOR`, before the transaction constants that
// follow it are ever reached. Measured on a device: `IWifiManager$Stub` carries 347 static fields
// whose values are mostly VALUE_INT but include that string, and `getConnectionInfo` is 98 there.
//
// A payload wider than four bytes is not an int either, and the byte count is what keeps a size
// field read out of a corrupt file from shifting past the end. Reading through 64 bits keeps every
// shift in range while counting down; `end` is what keeps a truncated payload from running off the
// end of the mapping.
static bool dexEncodedValue(const uint8_t** p, const uint8_t* end, int32_t* result, bool* isInt) {
    if (*p >= end) {
        return false;
    }
    const uint8_t header = *(*p)++;
    const uint32_t type = (uint32_t) (header & 0x1f);

    // The two tags whose value is the header byte itself. Returning here is what keeps the walk in
    // step; `isInt` is false because neither is an int.
    if (type == FF_DEX_VALUE_NULL || type == FF_DEX_VALUE_BOOLEAN) {
        *result = 0;
        *isInt = false;
        return true;
    }

    const uint32_t size = (uint32_t) (header >> 5) + 1;
    if (size > 8) {
        return false;
    }

    uint64_t value = 0;
    for (uint32_t i = 0; i < size; ++i) {
        if (*p >= end) {
            return false;
        }
        if (i < sizeof(value)) {
            value |= (uint64_t) (*(*p)++) << (i * 8);
        } else {
            (void) *(*p)++;
        }
    }

    *isInt = type == FF_DEX_VALUE_INT && size <= sizeof(uint32_t);
    if (*isInt) {
        // VALUE_INT is sign-extended from its own width, not from 32 bits: a one-byte -1 is written
        // as `21 ff`, one byte of 0xff, and has to come back as -1 rather than 255.
        const uint32_t bits = size * 8;
        if ((value & (1ull << (bits - 1))) != 0) {
            value |= ~((1ull << bits) - 1);
        }
    }
    *result = (int32_t) (uint32_t) value;
    return true;
}

// string_data_item: a uleb128 length in UTF-16 code units, then MUTF-8 bytes and a NUL terminator.
// The descriptors and field names looked up here are ASCII, where MUTF-8 and UTF-8 agree, so the
// bytes can be used as they are.
static const char* dexString(const uint8_t* dex, const uint8_t* end, uint32_t index) {
    const uint8_t* ids = dex + dexU32(dex + FF_DEX_OFF_STRING_IDS);
    // `string_ids_size` is not read: the check is that the entry `index` names lies inside the
    // mapping, which is what the read below needs, and the table is the only thing that can say it
    // without trusting a second field out of the same file.
    if (!dexTableInRange(dex, end, ids, (uint64_t) index + 1, 4)) {
        return nullptr;
    }
    const uint8_t* p = dex + dexU32(ids + (size_t) index * 4);
    if (p < dex || p >= end) {
        return nullptr;
    }
    (void) dexUleb128(&p, end);
    if (p >= end) {
        return nullptr;
    }
    return memchr(p, '\0', (size_t) (end - p)) != nullptr ? (const char*) p : nullptr;
}

// One dex entry, validated. Every offset in the header comes out of the file, so each table is range
// checked before the walk uses it.
typedef struct FFDexTables {
    const uint8_t* dex;
    const uint8_t* end;
    const uint8_t* types;
    uint32_t typeCount;
    const uint8_t* classes;
    uint32_t classCount;
} FFDexTables;

static const char* dexOpen(const uint8_t* dex, size_t size, FFDexTables* tables) {
    if (size < FF_DEX_HEADER_SIZE || dexU32(dex + FF_DEX_OFF_ENDIAN_TAG) != FF_DEX_ENDIAN_TAG) {
        return "Not a dex file";
    }
    const uint32_t fileSize = dexU32(dex + FF_DEX_OFF_FILE_SIZE);
    if (fileSize > size || fileSize < FF_DEX_HEADER_SIZE) {
        return "The dex file size is out of range";
    }
    const uint8_t* end = dex + fileSize;

    const uint8_t* types = dex + dexU32(dex + FF_DEX_OFF_TYPE_IDS);
    const uint32_t typeCount = dexU32(dex + FF_DEX_OFF_TYPE_IDS_SIZE);
    if (!dexTableInRange(dex, end, types, typeCount, 4)) {
        return "The dex type table is out of range";
    }

    const uint8_t* classes = dex + dexU32(dex + FF_DEX_OFF_CLASS_DEFS);
    const uint32_t classCount = dexU32(dex + FF_DEX_OFF_CLASS_DEFS_SIZE);
    if (!dexTableInRange(dex, end, classes, classCount, FF_DEX_CLASS_DEF_SIZE)) {
        return "The dex class table is out of range";
    }

    tables->dex = dex;
    tables->end = end;
    tables->types = types;
    tables->typeCount = typeCount;
    tables->classes = classes;
    tables->classCount = classCount;
    return nullptr;
}

// One pass over the type table, resolving the type index of every class that still has a request
// open. This pass is what a lookup costs: `framework.jar`'s classes.dex lists 8428 descriptors, each
// one a `string_data_item` read from a different part of a 51 MB mapping. Matching every request
// against the same pass is what makes a batch cheaper than the sum of its fields -- the descriptor is
// read once however many classes are being looked for, and the pass ends as soon as the last request
// has an index rather than at the end of the table.
//
// `typeIndexes` is parallel to `requests` and rebuilt here rather than kept across entries: a type
// index is a property of the entry's own table, and the next entry numbers its types differently.
static void dexFindTypes(const FFDexTables* tables, const FFDexStaticIntRequest* requests, const uint8_t* settled, uint32_t* typeIndexes, uint32_t count) {
    uint32_t remaining = 0;
    for (uint32_t r = 0; r < count; ++r) {
        typeIndexes[r] = UINT32_MAX;
        if (!settled[r]) {
            ++remaining;
        }
    }

    for (uint32_t i = 0; i < tables->typeCount && remaining > 0; ++i) {
        const char* descriptor = dexString(tables->dex, tables->end, dexU32(tables->types + (size_t) i * 4));
        if (descriptor == nullptr) {
            continue;
        }
        for (uint32_t r = 0; r < count; ++r) {
            if (settled[r] || typeIndexes[r] != UINT32_MAX || strcmp(descriptor, requests[r].classDescriptor) != 0) {
                continue;
            }
            typeIndexes[r] = i;
            --remaining;
        }
    }
}

// Walks the static field list of the class that `typeIndex` names and the class's static value array
// in step, writing the value of every request for that class whose field name matches. Both lists are
// ordered by field index and cover exactly the same fields -- when they do not, the pairing is not
// trustworthy and nothing is written.
//
// `defined` says whether this dex *defines* the class, which is not the same as mentioning it: a dex
// lists every type it references in its type table, so a class that a later entry of the jar defines
// still has a type index here, with no class_def_item behind it. Only a class_def_item makes the
// answer this dex's; without one the caller moves on to the next entry.
//
// A request this class cannot answer -- a field it does not declare, or one it declares as something
// other than an int -- is closed here rather than reported: the class is this dex's, so no later
// entry of the jar could give it a different answer, and one field the caller can live without must
// not hold back the rest of the batch. It is closed with the sentinel, and what this returns is only
// ever a dex that cannot be read, which is the one thing no later entry can fix.
static const char* dexClassStaticInts(const FFDexTables* tables, uint32_t typeIndex, const FFDexStaticIntRequest* requests, const uint32_t* typeIndexes, uint8_t* settled, uint32_t count, bool* defined) {
    *defined = false;
    const uint8_t* dex = tables->dex;
    const uint8_t* end = tables->end;

    for (uint32_t i = 0; i < tables->classCount; ++i) {
        const uint8_t* classDef = tables->classes + (size_t) i * FF_DEX_CLASS_DEF_SIZE;
        if (dexU32(classDef) != typeIndex) {
            continue;
        }
        // Defined here, so from this point on the answer is this dex's, whatever it turns out to be.
        *defined = true;

        const uint8_t* fields = dex + dexU32(classDef + FF_DEX_OFF_CLASS_DEF_DATA);
        const uint8_t* values = dex + dexU32(classDef + FF_DEX_OFF_CLASS_DEF_STATIC_VALUES);
        // One byte each, so that the first uleb128 below has something to read.
        if (!dexInRange(dex, end, fields, 1) || !dexInRange(dex, end, values, 1)) {
            return "The dex class data is out of range";
        }

        const uint32_t staticFields = dexUleb128(&fields, end);
        (void) dexUleb128(&fields, end); // instance_fields_size
        (void) dexUleb128(&fields, end); // direct_methods_size
        (void) dexUleb128(&fields, end); // virtual_methods_size
        const uint32_t staticValues = dexUleb128(&values, end);
        if (staticFields != staticValues) {
            return "The dex static fields and values do not pair up";
        }

        const uint8_t* fieldIds = dex + dexU32(dex + FF_DEX_OFF_FIELD_IDS);
        uint32_t fieldIndex = 0;
        for (uint32_t j = 0; j < staticFields; ++j) {
            // The two arrays are walked in step, so running out of either one is a truncation.
            if (fields >= end || values >= end) {
                return "The dex static field list is truncated";
            }

            const uint32_t fieldIndexDiff = dexUleb128(&fields, end); // field_idx_diff
            if (fieldIndexDiff > UINT32_MAX - fieldIndex) {
                return "The dex field index is out of range";
            }
            fieldIndex += fieldIndexDiff;
            (void) dexUleb128(&fields, end); // access_flags
            int32_t value;
            bool isInt;
            if (!dexEncodedValue(&values, end, &value, &isInt)) {
                return "The dex static value list is truncated";
            }

            if (!dexTableInRange(dex, end, fieldIds, (uint64_t) fieldIndex + 1, FF_DEX_FIELD_ID_SIZE)) {
                return "The dex field table is out of range";
            }
            const char* name = dexString(dex, end, dexU32(fieldIds + (size_t) fieldIndex * FF_DEX_FIELD_ID_SIZE + FF_DEX_OFF_FIELD_ID_NAME));
            if (name == nullptr) {
                continue;
            }
            for (uint32_t r = 0; r < count; ++r) {
                if (settled[r] || typeIndexes[r] != typeIndex || strcmp(name, requests[r].fieldName) != 0) {
                    continue;
                }
                if (!isInt) {
                    // The field exists but is not an int, so its value is not the constant asked
                    // for. Closed the way a field the class does not declare is, because it is this
                    // dex's answer too. The check is here rather than on the first non-int in the
                    // list because the list legitimately holds strings (`DESCRIPTOR`) and booleans.
                    *requests[r].result = FF_DEX_STATIC_INT_UNRESOLVED;
                    settled[r] = 1;
                    continue;
                }
                *requests[r].result = value;
                settled[r] = 1;
            }
        }

        // The class is defined here, so a request of its own that went unanswered is one whose field
        // the class does not declare as a static field. Closed here for the same reason as above.
        for (uint32_t r = 0; r < count; ++r) {
            if (!settled[r] && typeIndexes[r] == typeIndex) {
                *requests[r].result = FF_DEX_STATIC_INT_UNRESOLVED;
                settled[r] = 1;
            }
        }
        return nullptr;
    }
    // The descriptor is in this dex's type table but the class itself is defined by another entry of
    // the jar. That is not a failure: `defined` stays false and the caller reads the next entry.
    return nullptr;
}

// ---------------------------------------------------------------------------------------------
// Locating classes.dex
// ---------------------------------------------------------------------------------------------

typedef struct FFDexMapping {
    uint8_t* mapped;     // the jar, as mapped; nullptr once released
    size_t mappedSize;
    const uint8_t* data; // the dex bytes, inside `mapped`
    size_t size;
} FFDexMapping;

static void wrapDexMapping(FFDexMapping* mapping) {
    assert(mapping);
    if (mapping->mapped != nullptr) {
        munmap(mapping->mapped, mapping->mappedSize);
    }
}

// Locates an entry through the jar's central directory.
//
// The sizes and the compression method are read from the directory rather than from the entry's own
// local header, which is the more reliable of the two: a local header is allowed to leave its sizes
// at zero and put them in a data descriptor behind the payload, and an archive writer is free to
// choose that. Only the local header's two length fields are used, because they are what says where
// the payload begins -- the local extra field does not have to match the central one.
static const char* findDexEntry(const uint8_t* jar, size_t jarSize, const char* entry, const uint8_t** data, size_t* dataSize, uint32_t* uncompressedSize, uint16_t* method) {
    const size_t entryLength = strlen(entry);
    if (jarSize < FF_ZIP_EOCD_SIZE) {
        return "The jar is too small to be a zip";
    }

    // The record is the last thing in the file, so it is searched for backwards -- over at most the
    // 64 KiB a trailing comment can take, not over the whole archive.
    const size_t newest = jarSize - FF_ZIP_EOCD_SIZE;
    const size_t oldest = jarSize > FF_ZIP_EOCD_MAX_COMMENT + FF_ZIP_EOCD_SIZE
        ? jarSize - FF_ZIP_EOCD_MAX_COMMENT - FF_ZIP_EOCD_SIZE
        : 0;
    size_t eocd = SIZE_MAX;
    for (size_t i = newest;; --i) {
        if (memcmp(jar + i, FF_ZIP_EOCD_MAGIC, sizeof(FF_ZIP_EOCD_MAGIC) - 1) == 0) {
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
        FF_DEBUG("No end-of-central-directory record in the last %zu bytes of a %zu byte jar",
            newest - oldest + FF_ZIP_EOCD_SIZE, jarSize);
        return "The jar has no zip directory";
    }

    const uint32_t directorySize = dexU32(jar + eocd + FF_ZIP_OFF_EOCD_DIRECTORY_SIZE);
    const uint32_t directoryOffset = dexU32(jar + eocd + FF_ZIP_OFF_EOCD_DIRECTORY);
    if (directoryOffset > jarSize || directorySize > jarSize - directoryOffset) {
        return "The zip directory is out of range";
    }

    const size_t directoryEnd = (size_t) directoryOffset + directorySize;
    for (size_t offset = directoryOffset; offset + FF_ZIP_CENTRAL_SIZE <= directoryEnd;) {
        if (memcmp(jar + offset, FF_ZIP_CENTRAL_MAGIC, sizeof(FF_ZIP_CENTRAL_MAGIC) - 1) != 0) {
            return "A zip directory entry is malformed";
        }
        const uint16_t nameLength = dexU16(jar + offset + FF_ZIP_OFF_CENTRAL_NAME_LENGTH);
        const uint16_t extraLength = dexU16(jar + offset + FF_ZIP_OFF_CENTRAL_EXTRA_LENGTH);
        const uint16_t commentLength = dexU16(jar + offset + FF_ZIP_OFF_CENTRAL_COMMENT_LENGTH);
        const size_t entrySize = FF_ZIP_CENTRAL_SIZE + (size_t) nameLength + (size_t) extraLength + (size_t) commentLength;
        if (entrySize > directoryEnd - offset) {
            FF_DEBUG("A zip entry claims %zu bytes but only %zu remain in the directory",
                entrySize, (size_t) (directoryEnd - offset));
            return "A zip directory entry runs past the directory";
        }

        if (nameLength == entryLength && memcmp(jar + offset + FF_ZIP_CENTRAL_SIZE, entry, entryLength) == 0) {
            const uint32_t compressedSize = dexU32(jar + offset + FF_ZIP_OFF_CENTRAL_COMPRESSED_SIZE);
            const uint32_t uncompressed = dexU32(jar + offset + FF_ZIP_OFF_CENTRAL_UNCOMPRESSED_SIZE);
            const uint32_t localOffset = dexU32(jar + offset + FF_ZIP_OFF_CENTRAL_LOCAL_OFFSET);
            if (compressedSize == FF_ZIP_ZIP64_SENTINEL || uncompressed == FF_ZIP_ZIP64_SENTINEL || localOffset == FF_ZIP_ZIP64_SENTINEL) {
                FF_DEBUG("The dex entry is zip64: compressed=%u, uncompressed=%u, localOffset=%u",
                    compressedSize, uncompressed, localOffset);
                return "A dex entry is stored as zip64, which is not supported";
            }
            if (localOffset > jarSize || FF_ZIP_LOCAL_HEADER_SIZE > jarSize - localOffset) {
                return "A zip local header is out of range";
            }

            const uint16_t localNameLength = dexU16(jar + localOffset + FF_ZIP_OFF_NAME_LENGTH);
            const uint16_t localExtraLength = dexU16(jar + localOffset + FF_ZIP_OFF_EXTRA_LENGTH);
            const size_t headerSize = FF_ZIP_LOCAL_HEADER_SIZE + (size_t) localNameLength + (size_t) localExtraLength;
            if (headerSize > jarSize - localOffset || compressedSize > jarSize - localOffset - headerSize) {
                return "The zip entry bounds are out of range";
            }

            *method = dexU16(jar + offset + FF_ZIP_OFF_CENTRAL_METHOD);
            *uncompressedSize = uncompressed;
            *data = jar + localOffset + headerSize;
            *dataSize = compressedSize;
            return nullptr;
        }

        offset += entrySize;
    }
    FF_DEBUG("No entry in the %zu byte jar is named %s", jarSize, entry);
    return "The jar has no usable dex entry";
}

// Fallback for a jar whose entry is STORED but whose header has no sizes: the dex magic is then a
// literal run of bytes in the file, and the header that follows it validates or it does not. It
// only ever locates the *first* dex in the jar, so it is a fallback for `classes.dex` alone.
static const uint8_t* findDexMagic(const uint8_t* jar, size_t jarSize, size_t* dexSize) {
    for (size_t i = 0; i + FF_DEX_HEADER_SIZE <= jarSize; ++i) {
        if (memcmp(jar + i, FF_DEX_MAGIC, sizeof(FF_DEX_MAGIC) - 1) != 0) {
            continue;
        }
        const uint8_t* dex = jar + i;
        const uint32_t fileSize = dexU32(dex + FF_DEX_OFF_FILE_SIZE);
        if (dexU32(dex + FF_DEX_OFF_ENDIAN_TAG) != FF_DEX_ENDIAN_TAG) {
            continue;
        }
        if (fileSize < FF_DEX_HEADER_SIZE || fileSize > jarSize - i) {
            continue;
        }
        *dexSize = fileSize;
        return dex;
    }
    return nullptr;
}

#ifdef FF_HAVE_ZLIB
static const char* inflateDex(const uint8_t* data, size_t dataSize, uint32_t uncompressedSize, uint8_t** out) {
    FF_LIBRARY_LOAD(zlib, "dlopen(libz) failed", "libz" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL(zlib, inflateInit2_, "dlsym(inflateInit2_) failed")
    FF_LIBRARY_LOAD_SYMBOL(zlib, inflate, "dlsym(inflate) failed")
    FF_LIBRARY_LOAD_SYMBOL(zlib, inflateEnd, "dlsym(inflateEnd) failed")

    // Released by the cleanup on every path out below; ownership moves to the caller by clearing
    // the pointer, which is also why no failure path has to free it by hand.
    FF_AUTO_FREE uint8_t* buffer = malloc(uncompressedSize);
    if (buffer == nullptr) {
        FF_DEBUG("malloc(%u) for the inflated dex failed", uncompressedSize);
        return "malloc failed";
    }

    // `uncompress` is not usable here: a zip entry holds a raw deflate stream, without the two byte
    // zlib header that entry point insists on. A negative window size tells inflate to skip the
    // header, which is what the zip format expects.
    z_stream stream = {};
    stream.next_in = (Bytef*) data;
    stream.avail_in = (uInt) dataSize;
    stream.next_out = buffer;
    stream.avail_out = (uInt) uncompressedSize;

    const int initStatus = ffinflateInit2_(&stream, -MAX_WBITS, ZLIB_VERSION, (int) sizeof(z_stream));
    if (initStatus != Z_OK) {
        FF_DEBUG("inflateInit2() failed: zlib status %d", initStatus);
        return "inflateInit2 failed";
    }
    const int status = ffinflate(&stream, Z_FINISH);
    ffinflateEnd(&stream);
    if (status != Z_STREAM_END || stream.total_out != (uLong) uncompressedSize) {
        FF_DEBUG("inflate() returned %d, produced %lu of %u bytes",
            status, stream.total_out, uncompressedSize);
        return "Inflating the dex failed";
    }

    *out = buffer;
    buffer = nullptr; // the caller owns it from here
    return nullptr;
}
#endif

const char* ffDexStaticInts(const char* jarPath, const FFDexStaticIntRequest* requests, uint32_t count) {
    // Every request is written the sentinel before anything can fail, so that a caller reads a result
    // rather than whatever its own stack held there, however this returns. Nothing below has to
    // remember to do it on its way out.
    for (uint32_t r = 0; r < count; ++r) {
        *requests[r].result = FF_DEX_STATIC_INT_UNRESOLVED;
    }
    if (count == 0) {
        return nullptr;
    }

    [[gnu::cleanup(wrapDexMapping)]] FFDexMapping mapping = {};

    {
        FF_AUTO_CLOSE_FD const int fd = open(jarPath, O_RDONLY | O_CLOEXEC);
        if (fd < 0) {
            FF_DEBUG("open(%s) failed: %s", jarPath, strerror(errno));
            return "open(jar) failed";
        }

        struct stat st = {};
        const int statStatus = fstat(fd, &st);
        if (statStatus != 0 || st.st_size <= 0) {
            FF_DEBUG("fstat(%s) reported %s (%lld bytes)", jarPath,
                statStatus != 0 ? strerror(errno) : "an empty file", (long long) st.st_size);
            return "fstat(jar) failed";
        }
        mapping.mappedSize = (size_t) st.st_size;
        mapping.mapped = mmap(nullptr, mapping.mappedSize, PROT_READ, MAP_PRIVATE, fd, 0);
        // The mapping outlives the descriptor, so the descriptor can go either way from here.
        if (mapping.mapped == MAP_FAILED) {
            FF_DEBUG("mmap(%s, %zu bytes) failed: %s", jarPath, mapping.mappedSize, strerror(errno));
            mapping.mapped = nullptr;
            return "mmap(jar) failed";
        }
    }

    // One byte per request saying whether it is closed -- answered, or given up on -- and the type
    // index it was found under in the entry being walked. Both are sized from `count` rather than
    // from a fixed bound, and both are released by their cleanup however this function returns.
    FF_AUTO_FREE uint8_t* settled = calloc(count, 1);
    FF_AUTO_FREE uint32_t* typeIndexes = malloc((size_t) count * sizeof(uint32_t));
    if (settled == nullptr || typeIndexes == nullptr) {
        FF_DEBUG("Allocating %zu bytes for %u static int requests failed",
            (size_t) count * (1 + sizeof(uint32_t)), count);
        return "malloc failed";
    }

    // A jar spreads its classes over `classes.dex`, `classes2.dex`, `classes3.dex`, ... and the
    // class being looked for is in whichever one the build put it in: `IAudioService$Stub` is in the
    // second dex of framework.jar, while `IWifiManager$Stub` is in the first of framework-wifi.jar.
    // The entries are therefore tried in order, and the first one that defines the class answers.
    // The count is a bound on the walk, not a claim about the jar -- AOSP has never shipped more
    // than a handful, and a class that is in none of them is left unanswered either way.
    //
    // The walk ends as soon as the last request is closed, so an entry that holds every class asked
    // for -- which is what the modules here ask for, one class per interface -- costs one entry read
    // and one pass over its type table.
    uint32_t pending = count;
    const char* error = nullptr;
    for (uint32_t index = 1; index <= FF_DEX_MAX_ENTRIES && pending > 0; ++index) {
        char entry[sizeof("classes999.dex")];
        if (index == 1) {
            memcpy(entry, FF_DEX_ENTRY, sizeof(FF_DEX_ENTRY));
        } else {
            (void) snprintf(entry, sizeof(entry), "classes%u.dex", index);
        }

        const uint8_t* data = nullptr;
        size_t dataSize = 0;
        uint32_t uncompressedSize = 0;
        uint16_t method = FF_ZIP_METHOD_STORED;
        // The decompressed dex, when the entry needed one. The cleanup releases it at the end of
        // the iteration, so every entry gets a buffer of its own and no way out of the loop -- an
        // early return included -- can leak one.
        FF_AUTO_FREE uint8_t* inflated = nullptr;

        if (findDexEntry(mapping.mapped, mapping.mappedSize, entry, &data, &dataSize, &uncompressedSize, &method) != nullptr) {
            if (index > 1) {
                // The magic scan only ever finds the first dex, so it cannot stand in for a later
                // entry: running past the end of the entries that exist ends the walk.
                break;
            }
            data = findDexMagic(mapping.mapped, mapping.mappedSize, &dataSize);
            if (data == nullptr) {
                FF_DEBUG("Neither %s nor a dex magic signature is in the %zu byte jar", entry, mapping.mappedSize);
                return "No dex in the jar";
            }
        } else if (method == FF_ZIP_METHOD_DEFLATED) {
            #ifdef FF_HAVE_ZLIB
            error = inflateDex(data, dataSize, uncompressedSize, &inflated);
            if (error != nullptr) {
                return error;
            }
            data = inflated;
            dataSize = uncompressedSize;
            #else
            return "The jar deflates its dex entries and fastfetch was built without zlib";
            #endif
        } else if (method != FF_ZIP_METHOD_STORED) {
            FF_DEBUG("%s uses compression method %u, only %u (stored) and %u (deflated) are handled",
                entry, method, FF_ZIP_METHOD_STORED, FF_ZIP_METHOD_DEFLATED);
            return "A dex entry uses an unsupported compression method";
        }
        // A STORED entry needs no further work: `dataSize` already holds the `compressedSize` that
        // `findDexEntry` checked against the mapping, and for a STORED entry the payload is the file
        // itself. The header's `uncompressedSize` is deliberately not used for it -- nothing bounds
        // that value, so a corrupt one reaches past the end of the mapping, which is exactly what the
        // dex header would then be validated against.

        mapping.data = data;
        mapping.size = dataSize;

        FFDexTables tables = {};
        error = dexOpen(mapping.data, mapping.size, &tables);
        if (error != nullptr) {
            // The dex cannot be read at all, which no later entry can fix.
            return error;
        }

        dexFindTypes(&tables, requests, settled, typeIndexes, count);

        // Every class this entry defines is walked once, however many of its fields were asked for.
        // A request whose class is not in this entry's type table keeps `UINT32_MAX` and is left for
        // the next entry; one whose class is only referenced here is left the same way by the walk
        // below, which is what `defined` reports.
        for (uint32_t r = 0; r < count; ++r) {
            if (settled[r] || typeIndexes[r] == UINT32_MAX) {
                continue;
            }
            const uint32_t typeIndex = typeIndexes[r];
            bool defined = false;
            error = dexClassStaticInts(&tables, typeIndex, requests, typeIndexes, settled, count, &defined);
            if (error != nullptr) {
                // The dex cannot be read, which no later entry can fix.
                return error;
            }
            if (!defined) {
                // Only referenced here, so the class is left for the next entry -- and the requests
                // that share it are marked as walked, since one lookup of a class answers all of
                // them and the class table should not be scanned once per field.
                for (uint32_t s = r; s < count; ++s) {
                    if (typeIndexes[s] == typeIndex) {
                        typeIndexes[s] = UINT32_MAX;
                    }
                }
            }
        }

        pending = 0;
        for (uint32_t r = 0; r < count; ++r) {
            if (!settled[r]) {
                ++pending;
            }
        }
    }

    // What is still open is a class no entry of the jar defines. That is an answer like any other --
    // the results already hold the sentinel, written before the walk began -- so the only thing left
    // is to say so for a debug build, where the sentinel on its own does not tell "this build has no
    // such method" apart from "the reader went wrong".
    for (uint32_t r = 0; r < count; ++r) {
        if (*requests[r].result == FF_DEX_STATIC_INT_UNRESOLVED) {
            FF_DEBUG("\"%s\" is not a static int field of \"%s\"", requests[r].fieldName, requests[r].classDescriptor);
        }
    }
    return nullptr;
}
