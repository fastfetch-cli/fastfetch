#include "common/android/dex.h"
#include "common/debug.h"
#include "common/io.h"
#include "common/mallocHelper.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#ifdef FF_HAVE_ZLIB
    #include "common/zlib.h"
#endif

// Offsets into the dex header, from https://source.android.com/docs/core/runtime/dex-format.
#define FF_DEX_ENDIAN_TAG 0x12345678u
#define FF_DEX_HEADER_SIZE 0x70
#define FF_DEX_OFF_FILE_SIZE 0x20
#define FF_DEX_OFF_ENDIAN_TAG 0x28
#define FF_DEX_OFF_STRING_IDS 0x3C
#define FF_DEX_OFF_TYPE_IDS_SIZE 0x40
#define FF_DEX_OFF_TYPE_IDS 0x44
#define FF_DEX_OFF_FIELD_IDS_SIZE 0x50
#define FF_DEX_OFF_FIELD_IDS 0x54
#define FF_DEX_OFF_METHOD_IDS_SIZE 0x58
#define FF_DEX_OFF_METHOD_IDS 0x5C
#define FF_DEX_OFF_CLASS_DEFS_SIZE 0x60
#define FF_DEX_OFF_CLASS_DEFS 0x64

// class_def_item: [u32 class_idx][u32 access_flags][u32 superclass_idx][u32 interfaces_off]
// [u32 source_file_idx][u32 annotations_off][u32 class_data_off][u32 static_values_off]
#define FF_DEX_CLASS_DEF_SIZE 32
#define FF_DEX_OFF_CLASS_DEF_DATA 24
#define FF_DEX_OFF_CLASS_DEF_STATIC_VALUES 28

// field_id_item: [u16 class_idx][u16 type_idx][u32 name_idx]
#define FF_DEX_FIELD_ID_SIZE 8
#define FF_DEX_OFF_FIELD_ID_TYPE 2
#define FF_DEX_OFF_FIELD_ID_NAME 4

// method_id_item: [u16 class_idx][u16 proto_idx][u32 name_idx]
#define FF_DEX_METHOD_ID_SIZE 8
#define FF_DEX_OFF_METHOD_ID_NAME 4

// code_item: [u16 registers_size][u16 ins_size][u16 outs_size][u16 tries_size][u32 debug_info_off]
// [u32 insns_size][u16 insns...]. Everything before the instruction stream is skipped by reading
// `insns_size` and starting `insns` where it ends.
#define FF_DEX_OFF_CODE_INSNS_SIZE 12
#define FF_DEX_OFF_CODE_INSNS 16

// The seven `iget*` opcodes, which are how a method reads an instance field. `iput*` shares the same
// format and the same operand, and a `writeToParcel` does not store to a field, so one range covers
// both and the walk does not have to tell them apart.
#define FF_DEX_OP_FIELD_FIRST 0x52
#define FF_DEX_OP_FIELD_LAST 0x5F

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
//
// `fieldIds` and `methodIds` are the two tables a class's own lists are resolved through: a
// class_data_item names its members by index into them, and the `name_idx` of the entry is what says
// which member it is. Holding them here rather than re-reading the header offset per class also
// turns the per-member bounds check into a count comparison, which has to happen before the index is
// multiplied out anyway -- on a 32-bit build `index * 8` wraps for a large enough index and the
// range check that follows passes again.
//
// `string_ids` is deliberately not held here: `dexString` checks the one entry it reads, and a table
// nothing walks in order does not need a second check.
typedef struct FFDexTables {
    const uint8_t* dex;
    const uint8_t* end;
    const uint8_t* types;
    uint32_t typeCount;
    const uint8_t* fieldIds;
    uint32_t fieldIdCount;
    const uint8_t* methodIds;
    uint32_t methodIdCount;
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

    const uint8_t* fieldIds = dex + dexU32(dex + FF_DEX_OFF_FIELD_IDS);
    const uint32_t fieldIdCount = dexU32(dex + FF_DEX_OFF_FIELD_IDS_SIZE);
    if (!dexTableInRange(dex, end, fieldIds, fieldIdCount, FF_DEX_FIELD_ID_SIZE)) {
        return "The dex field table is out of range";
    }

    const uint8_t* methodIds = dex + dexU32(dex + FF_DEX_OFF_METHOD_IDS);
    const uint32_t methodIdCount = dexU32(dex + FF_DEX_OFF_METHOD_IDS_SIZE);
    if (!dexTableInRange(dex, end, methodIds, methodIdCount, FF_DEX_METHOD_ID_SIZE)) {
        return "The dex method table is out of range";
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
    tables->fieldIds = fieldIds;
    tables->fieldIdCount = fieldIdCount;
    tables->methodIds = methodIds;
    tables->methodIdCount = methodIdCount;
    tables->classes = classes;
    tables->classCount = classCount;
    return nullptr;
}

// The descriptor a type index names, e.g. "I", "[F" or "Landroid/os/Parcel;".
static const char* dexTypeString(const FFDexTables* tables, uint32_t typeIndex) {
    if (typeIndex >= tables->typeCount) {
        return nullptr;
    }
    return dexString(tables->dex, tables->end, dexU32(tables->types + (size_t) typeIndex * 4));
}

// One pass over the type table, resolving the type index of every class that still has a request
// open. This pass is what a lookup costs: `framework.jar`'s classes.dex lists 8428 descriptors, each
// one a `string_data_item` read from a different part of a 51 MB mapping. Matching every request
// against the same pass is what makes a batch cheaper than the sum of its fields -- the descriptor is
// read once however many classes are being looked for, and the pass ends as soon as the last request
// has an index rather than at the end of the table.
//
// `typeIndexes` is parallel to `descriptors` and rebuilt here rather than kept across entries: a
// type index is a property of the entry's own table, and the next entry numbers its types
// differently.
static void dexFindTypes(const FFDexTables* tables, const char* const* descriptors, const uint8_t* settled, uint32_t* typeIndexes, uint32_t count) {
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
            if (settled[r] || typeIndexes[r] != UINT32_MAX || strcmp(descriptor, descriptors[r]) != 0) {
                continue;
            }
            typeIndexes[r] = i;
            --remaining;
        }
    }
}

// The lists of a class_data_item, each positioned at its first entry.
//
// A class_data_item is `[uleb static_fields_size][uleb instance_fields_size][uleb direct_methods_size]
// [uleb virtual_methods_size]` followed by the four lists in that order, so the start of each list is
// only reachable by stepping over the one in front of it. That is done once here for the three
// readers that each want a different list: the static field reader pairs the first with the class's
// static value array, the write-sequence reader walks the methods, and the field-existence reader
// walks the second.
//
// A field entry is `[uleb field_idx_diff][uleb access_flags]` and a method entry is the same plus a
// trailing `[uleb code_off]`, which is the offset of the code_item holding its instructions -- or 0
// for a method with no body, an abstract or native one.
//
// Each of the four lists is delta-encoded from zero on its own, so the counts are kept apart rather
// than summed: a class routinely has a direct method and a virtual one under the same index, and one
// accumulator carried across the two method lists reads the second list's names out of the wrong end
// of the method table. The field lists are read the same way, which is why the reader of each one
// starts its index at zero rather than continuing from the list in front of it.
typedef struct FFDexClassData {
    const uint8_t* staticFields;
    const uint8_t* instanceFields;
    const uint8_t* methods;
    uint32_t staticFieldCount;
    uint32_t instanceFieldCount;
    uint32_t directMethodCount;
    uint32_t virtualMethodCount;
} FFDexClassData;

// Steps over `count` field entries. A list that ends before its own count is a file that is not what
// it claims, not a short list: the count and the entries both come out of the same file.
static bool dexSkipFields(const uint8_t** p, const uint8_t* end, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        if (*p >= end) {
            return false;
        }
        (void) dexUleb128(p, end); // field_idx_diff
        (void) dexUleb128(p, end); // access_flags
    }
    return true;
}

static const char* dexReadClassData(const FFDexTables* tables, const uint8_t* classDef, FFDexClassData* data) {
    const uint8_t* dex = tables->dex;
    const uint8_t* end = tables->end;
    const uint32_t classDataOff = dexU32(classDef + FF_DEX_OFF_CLASS_DEF_DATA);
    if (classDataOff == 0) {
        // A class with no class_data_item declares nothing of its own -- its members are all
        // inherited. That is four empty lists, not a file that cannot be read, and the counts below
        // are what carry the answer.
        data->staticFields = nullptr;
        data->instanceFields = nullptr;
        data->methods = nullptr;
        data->staticFieldCount = 0;
        data->instanceFieldCount = 0;
        data->directMethodCount = 0;
        data->virtualMethodCount = 0;
        return nullptr;
    }

    const uint8_t* p = dex + classDataOff;
    // One byte, so that the first uleb128 has something to read.
    if (!dexInRange(dex, end, p, 1)) {
        return "The dex class data is out of range";
    }

    data->staticFieldCount = dexUleb128(&p, end);
    data->instanceFieldCount = dexUleb128(&p, end);
    data->directMethodCount = dexUleb128(&p, end);
    data->virtualMethodCount = dexUleb128(&p, end);

    data->staticFields = p;
    if (!dexSkipFields(&p, end, data->staticFieldCount)) {
        return "The dex class data is truncated";
    }
    data->instanceFields = p;
    if (!dexSkipFields(&p, end, data->instanceFieldCount)) {
        return "The dex class data is truncated";
    }
    data->methods = p;
    return nullptr;
}

// The class_def_item of the class that `typeIndex` names, or nullptr when this dex only references
// the class.
//
// Having a type index is not the same as being defined here: a dex lists every type it references in
// its type table, so a class that a later entry of the jar defines still has a type index in this
// one, with no class_def_item behind it. Only a class_def_item makes the answer this dex's, and
// without one the caller moves on to the next entry.
static const uint8_t* dexFindClassDef(const FFDexTables* tables, uint32_t typeIndex) {
    for (uint32_t i = 0; i < tables->classCount; ++i) {
        const uint8_t* classDef = tables->classes + (size_t) i * FF_DEX_CLASS_DEF_SIZE;
        if (dexU32(classDef) == typeIndex) {
            return classDef;
        }
    }
    return nullptr;
}

// Walks the static field list of the class that `typeIndex` names and the class's static value array
// in step, writing the value of every request for that class whose field name matches. Both lists are
// ordered by field index and cover exactly the same fields -- when they do not, the pairing is not
// trustworthy and nothing is written.
//
// `defined` says whether this dex *defines* the class, which the caller needs: a request the class
// cannot answer has to be left open when the class belongs to a later entry of the jar and closed
// when it does not.
//
// A request this class cannot answer -- a field it does not declare, or one it declares as something
// other than an int -- is closed here rather than reported: the class is this dex's, so no later
// entry of the jar could give it a different answer, and one field the caller can live without must
// not hold back the rest of the batch. It is closed with the sentinel, and what this returns is only
// ever a dex that cannot be read, which is the one thing no later entry can fix.
static const char* dexClassStaticInts(const FFDexTables* tables, uint32_t typeIndex, const FFDexStaticIntRequest* requests, const uint32_t* typeIndexes, uint8_t* settled, uint32_t count, bool* defined) {
    const uint8_t* classDef = dexFindClassDef(tables, typeIndex);
    *defined = classDef != nullptr;
    if (classDef == nullptr) {
        return nullptr;
    }

    const uint8_t* dex = tables->dex;
    const uint8_t* end = tables->end;

    FFDexClassData classData;
    const char* error = dexReadClassData(tables, classDef, &classData);
    if (error != nullptr) {
        return error;
    }

    // The static value array is parallel to the static field list and covers exactly the same fields,
    // which is what makes walking the two in step below meaningful. It is absent in two cases -- a
    // class with no static field, and a class none of whose static fields has a constant value -- and
    // the header writes 0 for it in both. The read is skipped rather than made against whatever byte
    // a zero offset points at, which is the dex magic and decodes as a four byte int.
    const uint8_t* fields = classData.staticFields;
    const uint8_t* values = dex;
    bool hasValues = false;
    if (classData.staticFieldCount > 0) {
        const uint32_t staticValuesOff = dexU32(classDef + FF_DEX_OFF_CLASS_DEF_STATIC_VALUES);
        if (staticValuesOff != 0) {
            values = dex + staticValuesOff;
            if (!dexInRange(dex, end, values, 1)) {
                return "The dex class data is out of range";
            }
            if (classData.staticFieldCount != dexUleb128(&values, end)) {
                return "The dex static fields and values do not pair up";
            }
            hasValues = true;
        }
    }

    uint32_t fieldIndex = 0;
    for (uint32_t j = 0; j < classData.staticFieldCount; ++j) {
        // The two arrays are walked in step, so running out of either one is a truncation.
        if (fields >= end || (hasValues && values >= end)) {
            return "The dex static field list is truncated";
        }

        const uint32_t fieldIndexDiff = dexUleb128(&fields, end); // field_idx_diff
        if (fieldIndexDiff > UINT32_MAX - fieldIndex) {
            return "The dex field index is out of range";
        }
        fieldIndex += fieldIndexDiff;
        (void) dexUleb128(&fields, end); // access_flags
        int32_t value = 0;
        // Left false when there is no value array, which closes the field with the sentinel below:
        // a class with no static value for the field has no answer for it.
        bool isInt = false;
        if (hasValues && !dexEncodedValue(&values, end, &value, &isInt)) {
            return "The dex static value list is truncated";
        }

        if (fieldIndex >= tables->fieldIdCount) {
            return "The dex field table is out of range";
        }
        const char* name = dexString(dex, end, dexU32(tables->fieldIds + (size_t) fieldIndex * FF_DEX_FIELD_ID_SIZE + FF_DEX_OFF_FIELD_ID_NAME));
        if (name == nullptr) {
            continue;
        }
        for (uint32_t r = 0; r < count; ++r) {
            if (settled[r] || typeIndexes[r] != typeIndex || strcmp(name, requests[r].fieldName) != 0) {
                continue;
            }
            if (!isInt) {
                // The field exists but is not an int, so its value is not the constant asked for.
                // Closed the way a field the class does not declare is, because it is this dex's
                // answer too. The check is here rather than on the first non-int in the list because
                // the list legitimately holds strings (`DESCRIPTOR`) and booleans.
                *requests[r].result = FF_DEX_STATIC_INT_UNRESOLVED;
                settled[r] = 1;
                continue;
            }
            *requests[r].result = value;
            settled[r] = 1;
        }
    }

    // The class is defined here, so a request of its own that went unanswered is one whose field the
    // class does not declare as a static field. Closed here for the same reason as above.
    for (uint32_t r = 0; r < count; ++r) {
        if (!settled[r] && typeIndexes[r] == typeIndex) {
            *requests[r].result = FF_DEX_STATIC_INT_UNRESOLVED;
            settled[r] = 1;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------------------------
// Walking a method's instructions
// ---------------------------------------------------------------------------------------------

// The width of every instruction, in code units. This is ART's `Instruction::SizeInCodeUnits`, and
// the walk below cannot stay on the instruction grid without it -- a walk that leaves the grid still
// produces a plausible-looking list of fields, which is what makes this worth a table rather than a
// guess. Unused and odex-only opcodes are 1 rather than 0, so that a byte which is not an instruction
// at all moves the walk on instead of leaving it where it is.
static const uint8_t dexOpcodeSize[256] = {
    /* 00 */ 1, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 1, 1, 1, 1, 1,
    /* 10 */ 1, 1, 1, 2, 3, 2, 2, 3, 5, 2, 2, 3, 2, 1, 1, 2,
    /* 20 */ 2, 1, 2, 2, 3, 3, 3, 1, 1, 2, 3, 3, 3, 2, 2, 2,
    /* 30 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1,
    /* 40 */ 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* 50 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* 60 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3,
    /* 70 */ 3, 3, 3, 1, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1,
    /* 80 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 90 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* a0 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* b0 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* c0 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* d0 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* e0 */ 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* f0 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 4, 3, 3, 2, 2,
};

// The code_item at `codeOff`, with its instruction array range checked. nullptr is one that cannot
// be read: an offset outside the mapping, or an instruction count that runs past the end.
static const uint8_t* dexReadCode(const FFDexTables* tables, uint32_t codeOff) {
    const uint8_t* code = tables->dex + codeOff;
    if (!dexInRange(tables->dex, tables->end, code, FF_DEX_OFF_CODE_INSNS)) {
        return nullptr;
    }
    const uint32_t insnsSize = dexU32(code + FF_DEX_OFF_CODE_INSNS_SIZE);
    if (!dexTableInRange(tables->dex, tables->end, code + FF_DEX_OFF_CODE_INSNS, insnsSize, 2)) {
        return nullptr;
    }
    return code;
}

// Records `name` and `type` as the next field of a write sequence, unless the name is already in it.
//
// An array field is read twice: once for its length, which is the count written ahead of the elements,
// and again inside the loop that writes them. A class cannot declare two instance fields under one
// name, so a repeat is always that reload, and the first read is where the field belongs in the
// sequence.
//
// A name or descriptor longer than the caller's buffer is refused rather than truncated: half a name
// matches nothing, which is indistinguishable from the field not being in the class at all. Refusing
// one field leaves the whole sequence unwritten, because a caller walking a sequence with a hole in
// it would read every field behind the hole from the wrong place.
static bool dexParcelFieldAdd(FFDexParcelField* fields, uint32_t* count, uint32_t capacity, const char* name, const char* type) {
    for (uint32_t i = 0; i < *count; ++i) {
        if (strcmp(fields[i].name, name) == 0) {
            return true;
        }
    }
    if (*count >= capacity) {
        return false;
    }
    const size_t nameSize = strlen(name) + 1;
    const size_t typeSize = strlen(type) + 1;
    if (nameSize > FF_DEX_PARCEL_FIELD_NAME_MAX || typeSize > FF_DEX_PARCEL_FIELD_TYPE_MAX) {
        return false;
    }
    memcpy(fields[*count].name, name, nameSize);
    memcpy(fields[*count].type, type, typeSize);
    ++*count;
    return true;
}

// Walks the instruction grid of a code_item and records every instance field the method reads, in the
// order it reads them -- which is the order `writeToParcel` puts them on the wire.
//
// The grid is followed by opcode width, so the walk only ever lands on an instruction start, and it
// stops at the first payload pseudo-instruction. A `switch` or a `fill-array-data` leaves one behind
// every reachable instruction; its contents are data rather than instructions, and stepping into it
// desynchronises the grid for good.
//
// `iget*` and `iput*` share a format and put the field index in the code unit behind the opcode in
// both, which is what lets one test cover the whole range. A `writeToParcel` does not store to a
// field, so the `iput` half is never taken -- it is in the range because splitting it would be a
// claim about the method that the opcode does not make.
static const char* dexWalkParcelFields(const FFDexTables* tables, const uint8_t* code, FFDexParcelField* fields, uint32_t capacity, uint32_t* count) {
    *count = 0;
    const uint8_t* dex = tables->dex;
    const uint8_t* end = tables->end;
    const uint32_t insnsSize = dexU32(code + FF_DEX_OFF_CODE_INSNS_SIZE);
    const uint8_t* insns = code + FF_DEX_OFF_CODE_INSNS;

    for (uint32_t i = 0; i < insnsSize;) {
        const uint16_t unit = dexU16(insns + (size_t) i * 2);
        // The three payload pseudo-instructions put their ident in the high byte and zero in the low
        // one. `nop` is the one instruction whose low byte is zero as well, and it is told apart by
        // the whole code unit being zero. Testing the high byte alone is what gets this wrong: a
        // `sparse-switch` over register 2 encodes as 0x022c, which is an instruction, not a payload.
        if ((unit & 0xFF) == 0 && unit != 0) {
            break;
        }
        const uint32_t opcode = unit & 0xFF;
        if (opcode >= FF_DEX_OP_FIELD_FIRST && opcode <= FF_DEX_OP_FIELD_LAST) {
            if (i + 1 >= insnsSize) {
                break;
            }
            const uint32_t fieldIndex = dexU16(insns + ((size_t) i + 1) * 2);
            if (fieldIndex >= tables->fieldIdCount) {
                return "The dex field table is out of range";
            }
            const uint8_t* fieldId = tables->fieldIds + (size_t) fieldIndex * FF_DEX_FIELD_ID_SIZE;
            const char* name = dexString(dex, end, dexU32(fieldId + FF_DEX_OFF_FIELD_ID_NAME));
            const char* type = dexTypeString(tables, dexU16(fieldId + FF_DEX_OFF_FIELD_ID_TYPE));
            if (name == nullptr || type == nullptr) {
                return "The dex field is not resolvable";
            }
            if (!dexParcelFieldAdd(fields, count, capacity, name, type)) {
                FF_DEBUG("\"%s\" (%s) does not fit a sequence of %u fields", name, type, capacity);
                *count = 0;
                return nullptr;
            }
        }
        i += dexOpcodeSize[opcode];
    }
    return nullptr;
}

// Walks the instance field list of one `class_data_item` and writes `true` for every name it holds.
// The list is the caller's, so this answers one request's worth of names or several.
//
// This is the question a Parcelable whose layout is not field-driven needs: `DisplayCutout` reads
// everything through compiler-synthesised accessors, so the set of fields its class declares is the
// only thing that says how long its body is. It is asked as part of a write-sequence request rather
// than by a call of its own -- the list is already positioned by the `class_data_item` the sequence
// comes out of, and a separate call would repeat the whole walk to reach it.
static const char* dexMatchInstanceFields(const FFDexTables* tables, const uint8_t* fields, uint32_t fieldCount, const char* const* names, bool* results, uint32_t nameCount) {
    const uint8_t* dex = tables->dex;
    const uint8_t* end = tables->end;

    uint32_t fieldIndex = 0;
    for (uint32_t j = 0; j < fieldCount; ++j) {
        if (fields >= end) {
            return "The dex class data is truncated";
        }
        const uint32_t fieldIndexDiff = dexUleb128(&fields, end); // field_idx_diff
        if (fieldIndexDiff > UINT32_MAX - fieldIndex) {
            return "The dex field index is out of range";
        }
        fieldIndex += fieldIndexDiff;
        (void) dexUleb128(&fields, end); // access_flags

        if (fieldIndex >= tables->fieldIdCount) {
            return "The dex field table is out of range";
        }
        const char* name = dexString(dex, end, dexU32(tables->fieldIds + (size_t) fieldIndex * FF_DEX_FIELD_ID_SIZE + FF_DEX_OFF_FIELD_ID_NAME));
        if (name == nullptr) {
            continue;
        }
        for (uint32_t i = 0; i < nameCount; ++i) {
            if (!results[i] && strcmp(name, names[i]) == 0) {
                results[i] = true;
            }
        }
    }
    return nullptr;
}

// Walks the method list of the class that `typeIndex` names, and for every request whose method name
// it holds walks that method's instructions. `defined` is what `dexClassStaticInts` reports, for the
// same reason.
//
// A request the class does not answer is closed with a count of 0 -- the count a caller reads as
// "this build does not write that class", which is the truth: the class is this dex's, so no later
// entry of the jar has the method either.
static const char* dexClassParcelFields(const FFDexTables* tables, uint32_t typeIndex, const FFDexParcelRequest* requests, const uint32_t* typeIndexes, uint8_t* settled, uint32_t count, bool* defined) {
    const uint8_t* classDef = dexFindClassDef(tables, typeIndex);
    *defined = classDef != nullptr;
    if (classDef == nullptr) {
        return nullptr;
    }

    const uint8_t* dex = tables->dex;
    const uint8_t* end = tables->end;

    FFDexClassData classData;
    const char* error = dexReadClassData(tables, classDef, &classData);
    if (error != nullptr) {
        return error;
    }

    // The instance fields first, out of the list the `class_data_item` has already positioned. A
    // request that asked for them is answered here whether or not it also asked for a sequence, and
    // one that asked for neither is settled by the loop at the end like any other.
    for (uint32_t r = 0; r < count; ++r) {
        if (settled[r] || typeIndexes[r] != typeIndex || requests[r].instanceFieldCount == 0) {
            continue;
        }
        error = dexMatchInstanceFields(
            tables, classData.instanceFields, classData.instanceFieldCount,
            requests[r].instanceFieldNames, requests[r].instanceFieldResults, requests[r].instanceFieldCount
        );
        if (error != nullptr) {
            return error;
        }
    }

    const uint8_t* methods = classData.methods;
    // Two runs, each delta-encoded from zero on its own -- so the index is reset per run rather than
    // carried across, and a direct method and a virtual one under the same index both resolve.
    for (uint32_t list = 0; list < 2; ++list) {
        const uint32_t listCount = list == 0 ? classData.directMethodCount : classData.virtualMethodCount;
        uint32_t methodIndex = 0;
        for (uint32_t j = 0; j < listCount; ++j) {
            if (methods >= end) {
                return "The dex class data is truncated";
            }
            const uint32_t methodIndexDiff = dexUleb128(&methods, end); // method_idx_diff
            if (methodIndexDiff > UINT32_MAX - methodIndex) {
                return "The dex method index is out of range";
            }
            methodIndex += methodIndexDiff;
            (void) dexUleb128(&methods, end); // access_flags
            const uint32_t codeOff = dexUleb128(&methods, end);

            if (methodIndex >= tables->methodIdCount) {
                return "The dex method table is out of range";
            }
            const char* name = dexString(dex, end, dexU32(tables->methodIds + (size_t) methodIndex * FF_DEX_METHOD_ID_SIZE + FF_DEX_OFF_METHOD_ID_NAME));
            if (name == nullptr) {
                continue;
            }

            for (uint32_t r = 0; r < count; ++r) {
                // A request that left the sequence out is one that only wanted the instance fields,
                // which are already written; it has no method name to match against.
                if (settled[r] || typeIndexes[r] != typeIndex || requests[r].methodName == nullptr
                    || strcmp(name, requests[r].methodName) != 0) {
                    continue;
                }
                // A method with no body -- abstract, or native -- writes nothing, and the count stays 0.
                const uint8_t* code = codeOff == 0 ? nullptr : dexReadCode(tables, codeOff);
                if (codeOff != 0 && code == nullptr) {
                    return "The dex code is out of range";
                }
                if (code != nullptr) {
                    error = dexWalkParcelFields(tables, code, requests[r].fields, requests[r].capacity, requests[r].count);
                    if (error != nullptr) {
                        return error;
                    }
                }
                settled[r] = 1;
            }
        }
    }

    // The class is defined here, so a request of its own that went unanswered is one whose method the
    // class does not declare. The count it already holds is the 0 that says so.
    for (uint32_t r = 0; r < count; ++r) {
        if (!settled[r] && typeIndexes[r] == typeIndex) {
            settled[r] = 1;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------------------------
// Locating classes.dex
// ---------------------------------------------------------------------------------------------

typedef struct FFDexMapping {
    uint8_t* mapped; // the jar, as mapped; nullptr once released
    size_t mappedSize;
} FFDexMapping;

static void wrapDexMapping(FFDexMapping* mapping) {
    assert(mapping);
    if (mapping->mapped != nullptr) {
        munmap(mapping->mapped, mapping->mappedSize);
    }
}

// Maps the jar read-only. The pages the walk touches are the only ones faulted in, which is what
// keeps a 52 MB framework.jar off the heap and out of the cost of a lookup. The mapping outlives the
// descriptor, so the descriptor is closed before this returns.
static const char* dexMapJar(const char* jarPath, FFDexMapping* mapping) {
    FF_AUTO_CLOSE_FD int fd = open(jarPath, O_RDONLY | O_CLOEXEC);
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
    mapping->mappedSize = (size_t) st.st_size;
    mapping->mapped = mmap(nullptr, mapping->mappedSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapping->mapped == MAP_FAILED) {
        FF_DEBUG("mmap(%s, %zu bytes) failed: %s", jarPath, mapping->mappedSize, strerror(errno));
        mapping->mapped = nullptr;
        return "mmap(jar) failed";
    }
    return nullptr;
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
    const char* error = ffZlibLoad(FF_ZLIB_INFLATE);
    if (error != nullptr) {
        FF_DEBUG("The dex cannot be inflated: %s", error);
        return error;
    }

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

    const int initStatus = inflateInit2(&stream, -MAX_WBITS);
    if (initStatus != Z_OK) {
        FF_DEBUG("inflateInit2() failed: zlib status %d", initStatus);
        return "inflateInit2 failed";
    }
    const int status = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
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

// The dex bytes of one jar entry, and the validated tables of the dex they hold.
typedef struct FFDexEntry {
    const uint8_t* data;
    size_t size;
    FFDexTables tables;
} FFDexEntry;

// Locates `classesN.dex`, inflates it when the entry is deflated, and validates the header.
//
// `present` reports whether the jar holds the entry at all. A jar spreads its classes over as many
// entries as it needs and then simply runs out, so an entry that is not there ends the walk rather
// than failing it -- except for the first, which every jar has and which is where the magic scan
// stands in for a directory that cannot be read. `inflated` receives the buffer the caller has to
// release when the entry needed one, and is left alone otherwise.
static const char* dexOpenEntry(const FFDexMapping* mapping, uint32_t index, bool* present, uint8_t** inflated, FFDexEntry* entry) {
    // Wide enough for `classes999.dex`, which is the longest name `FF_DEX_MAX_ENTRIES` can ask for.
    char name[sizeof("classes999.dex")];
    if (index == 1) {
        memcpy(name, FF_DEX_ENTRY, sizeof(FF_DEX_ENTRY));
    } else {
        (void) snprintf(name, sizeof(name), "classes%u.dex", index);
    }
    *present = true;

    const uint8_t* data = nullptr;
    size_t dataSize = 0;
    uint32_t uncompressedSize = 0;
    uint16_t method = FF_ZIP_METHOD_STORED;
    if (findDexEntry(mapping->mapped, mapping->mappedSize, name, &data, &dataSize, &uncompressedSize, &method) != nullptr) {
        if (index > 1) {
            // The magic scan only ever finds the first dex, so it cannot stand in for a later entry:
            // running past the end of the entries that exist ends the walk.
            *present = false;
            return nullptr;
        }
        data = findDexMagic(mapping->mapped, mapping->mappedSize, &dataSize);
        if (data == nullptr) {
            FF_DEBUG("Neither %s nor a dex magic signature is in the %zu byte jar", name, mapping->mappedSize);
            return "No dex in the jar";
        }
    } else if (method == FF_ZIP_METHOD_DEFLATED) {
        #ifdef FF_HAVE_ZLIB
        const char* error = inflateDex(data, dataSize, uncompressedSize, inflated);
        if (error != nullptr) {
            return error;
        }
        data = *inflated;
        dataSize = uncompressedSize;
        #else
        (void) inflated; // the parameter is only written on the inflating path
        return "The jar deflates its dex entries and fastfetch was built without zlib";
        #endif
    } else if (method != FF_ZIP_METHOD_STORED) {
        FF_DEBUG("%s uses compression method %u, only %u (stored) and %u (deflated) are handled",
            name, method, FF_ZIP_METHOD_STORED, FF_ZIP_METHOD_DEFLATED);
        return "A dex entry uses an unsupported compression method";
    }
    // A STORED entry needs no further work: `dataSize` already holds the `compressedSize` that
    // `findDexEntry` checked against the mapping, and for a STORED entry the payload is the file
    // itself. The header's `uncompressedSize` is deliberately not used for it -- nothing bounds that
    // value, so a corrupt one reaches past the end of the mapping, which is exactly what the dex
    // header would then be validated against.

    entry->data = data;
    entry->size = dataSize;
    return dexOpen(entry->data, entry->size, &entry->tables);
}

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
    const char* error = dexMapJar(jarPath, &mapping);
    if (error != nullptr) {
        return error;
    }

    // One byte per request saying whether it is closed -- answered, or given up on -- the type index
    // it was found under in the entry being walked, and the descriptor that index was found for. All
    // three are sized from `count` rather than from a fixed bound, and all three are released by
    // their cleanup however this function returns.
    FF_AUTO_FREE uint8_t* settled = calloc(count, 1);
    FF_AUTO_FREE uint32_t* typeIndexes = malloc((size_t) count * sizeof(uint32_t));
    FF_AUTO_FREE const char** descriptors = malloc((size_t) count * sizeof(const char*));
    if (settled == nullptr || typeIndexes == nullptr || descriptors == nullptr) {
        FF_DEBUG("Allocating the tables for %u static int requests failed", count);
        return "malloc failed";
    }
    for (uint32_t r = 0; r < count; ++r) {
        descriptors[r] = requests[r].classDescriptor;
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
    for (uint32_t index = 1; index <= FF_DEX_MAX_ENTRIES && pending > 0; ++index) {
        // The decompressed dex, when the entry needed one. The cleanup releases it at the end of
        // the iteration, so every entry gets a buffer of its own and no way out of the loop -- an
        // early return included -- can leak one.
        FF_AUTO_FREE uint8_t* inflated = nullptr;
        FFDexEntry entry = {};
        bool present = false;
        error = dexOpenEntry(&mapping, index, &present, &inflated, &entry);
        if (error != nullptr) {
            // The dex cannot be read at all, which no later entry can fix.
            return error;
        }
        if (!present) {
            break;
        }

        dexFindTypes(&entry.tables, descriptors, settled, typeIndexes, count);

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
            error = dexClassStaticInts(&entry.tables, typeIndex, requests, typeIndexes, settled, count, &defined);
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

const char* ffDexParcelFields(const char* jarPath, const FFDexParcelRequest* requests, uint32_t count) {
    // Every request is written its answers before anything can fail, so that a caller reads an answer
    // rather than whatever its own stack held there, however this returns. The instance fields go
    // first: `false` is what both "the class does not declare it" and "the jar cannot be read" mean,
    // and the walk below only ever turns one of them true.
    for (uint32_t r = 0; r < count; ++r) {
        if (requests[r].methodName != nullptr) {
            *requests[r].count = 0;
        }
        for (uint32_t i = 0; i < requests[r].instanceFieldCount; ++i) {
            requests[r].instanceFieldResults[i] = false;
        }
    }
    if (count == 0) {
        return nullptr;
    }

    [[gnu::cleanup(wrapDexMapping)]] FFDexMapping mapping = {};
    const char* error = dexMapJar(jarPath, &mapping);
    if (error != nullptr) {
        return error;
    }

    FF_AUTO_FREE uint8_t* settled = calloc(count, 1);
    FF_AUTO_FREE uint32_t* typeIndexes = malloc((size_t) count * sizeof(uint32_t));
    FF_AUTO_FREE const char** descriptors = malloc((size_t) count * sizeof(const char*));
    if (settled == nullptr || typeIndexes == nullptr || descriptors == nullptr) {
        FF_DEBUG("Allocating the tables for %u parcel field requests failed", count);
        return "malloc failed";
    }
    for (uint32_t r = 0; r < count; ++r) {
        descriptors[r] = requests[r].classDescriptor;
    }

    uint32_t pending = count;
    for (uint32_t index = 1; index <= FF_DEX_MAX_ENTRIES && pending > 0; ++index) {
        FF_AUTO_FREE uint8_t* inflated = nullptr;
        FFDexEntry entry = {};
        bool present = false;
        error = dexOpenEntry(&mapping, index, &present, &inflated, &entry);
        if (error != nullptr) {
            // The dex cannot be read at all, which no later entry can fix.
            return error;
        }
        if (!present) {
            break;
        }

        dexFindTypes(&entry.tables, descriptors, settled, typeIndexes, count);

        for (uint32_t r = 0; r < count; ++r) {
            if (settled[r] || typeIndexes[r] == UINT32_MAX) {
                continue;
            }
            const uint32_t typeIndex = typeIndexes[r];
            bool defined = false;
            error = dexClassParcelFields(&entry.tables, typeIndex, requests, typeIndexes, settled, count, &defined);
            if (error != nullptr) {
                return error;
            }
            if (!defined) {
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

    // A request still holding 0 is one whose class no entry of the jar defines, or whose class does
    // not declare the method. Both are the same answer -- this build does not write that class -- and
    // for a debug build it is worth saying which of the two it was, since the count on its own does
    // not tell "this build has no such method" apart from "the reader went wrong". The same goes for
    // an instance field left `false`.
    for (uint32_t r = 0; r < count; ++r) {
        if (requests[r].methodName != nullptr && *requests[r].count == 0) {
            FF_DEBUG("The write sequence of \"%s.%s\" is not in the jar", requests[r].classDescriptor, requests[r].methodName);
        }
        for (uint32_t i = 0; i < requests[r].instanceFieldCount; ++i) {
            if (!requests[r].instanceFieldResults[i]) {
                FF_DEBUG("\"%s\" is not an instance field of \"%s\"", requests[r].instanceFieldNames[i], requests[r].classDescriptor);
            }
        }
    }
    return nullptr;
}
