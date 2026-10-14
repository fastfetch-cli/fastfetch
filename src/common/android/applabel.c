#include "common/android/applabel.h"
#include "common/android/binder.h"
#include "common/android/dex.h"
#include "common/android/zip.h"
#include "common/debug.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

// The package manager, and the one method that describes an app to a caller that is not the shell.
// IPackageManager has always been part of framework.jar -- it is not one of the frameworks that
// moved into an APEX -- so there is one path and no fallback.
#define FF_ANDROID_LABEL_SERVICE "package"
#define FF_ANDROID_LABEL_DESCRIPTOR "android.content.pm.IPackageManager"
#define FF_ANDROID_LABEL_JAR "/system/framework/framework.jar"
#define FF_ANDROID_LABEL_STUB "Landroid/content/pm/IPackageManager$Stub;"
#define FF_ANDROID_LABEL_TRANSACTION "TRANSACTION_getApplicationInfo"

#define FF_ANDROID_LABEL_TABLE "resources.arsc"

// The longest package name AIDL accepts is 255 characters, and the request is one string16 of it, the
// interface token, a 64-bit flags argument and a 32-bit user id.
// clang-format off
#define FF_ANDROID_LABEL_REQUEST_SIZE \
    (12 + ((4 + 2 * (sizeof(FF_ANDROID_LABEL_DESCRIPTOR) - 1) + 3) & ~3u) \
        + ((4 + 2 * 255 + 3) & ~3u) + 8u + 4u)
// clang-format on

// An ApplicationInfo parcel runs 2.0 to 3.2 KB on the test device, and the surplus is what a reply
// with more split names and directories than any of those would spend.
#define FF_ANDROID_LABEL_REPLY_SIZE (16 * 1024)

// Long enough for the `/data/app/~~<22 base64 chars>==/<package>-<22 chars>==/base.apk` the platform
// installs to, with room to spare.
#define FF_ANDROID_LABEL_PATH_SIZE 512

// ---------------------------------------------------------------------------------------------
// The resource table
// ---------------------------------------------------------------------------------------------
//
// `resources.arsc` is a ResTable: a header, a global string pool, and then one ResTable_package per
// package it defines, each carrying its own pools for type names and entry keys followed by the
// types themselves. Resolving `labelRes` is two indices deep -- the package's number names the
// package and the type, the low half names the entry -- and every step below reads the file rather
// than a table of what the file is expected to hold, because the layout is the platform's and the
// numbers are per APK.

// ResChunk_header: type(2) headerSize(2) size(4). Every chunk in the file starts with one.
#define FF_ARSC_TABLE_TYPE 0x0002
#define FF_ARSC_STRING_POOL_TYPE 0x0001
#define FF_ARSC_TABLE_PACKAGE_TYPE 0x0200
#define FF_ARSC_TABLE_TYPE_TYPE 0x0201

// ResStringPool: header(8) stringCount(4) styleCount(4) flags(4) stringsStart(4) stylesStart(4)
#define FF_ARSC_POOL_HEADER_SIZE 28
#define FF_ARSC_OFF_POOL_CHUNK_SIZE 4
#define FF_ARSC_OFF_POOL_COUNT 8
#define FF_ARSC_OFF_POOL_FLAGS 16
#define FF_ARSC_OFF_POOL_STRINGS_START 20

// A UTF-8 pool says so with this bit, and then holds the strings as UTF-8 rather than as UTF-16.
#define FF_ARSC_POOL_FLAG_UTF8 0x0100

// ResTable_package: header(8) id(4) name(256, a UTF-16 array) typeStrings(4) lastPublicType(4) ...
#define FF_ARSC_OFF_PACKAGE_ID 8
#define FF_ARSC_OFF_PACKAGE_TYPE_STRINGS (8 + 4 + 256)

// ResTable_type: header(8) id(1) flags(1) reserved(2) entryCount(4) entriesStart(4) config(...)
#define FF_ARSC_OFF_TYPE_ID 8
#define FF_ARSC_OFF_TYPE_FLAGS 9
#define FF_ARSC_OFF_TYPE_ENTRY_COUNT 12
#define FF_ARSC_OFF_TYPE_ENTRIES_START 16
#define FF_ARSC_OFF_TYPE_CONFIG 20

// The two shapes a type can take that this does not read: a sparse type stores its offsets as
// index-offset pairs rather than as a flat table, and a 16-bit one stores them at half width. Neither
// is in the APKs measured, and both would resolve an entry to the wrong offset rather than to none,
// so they are refused rather than read as if they were the plain shape.
#define FF_ARSC_TYPE_FLAG_SPARSE 0x01
#define FF_ARSC_TYPE_FLAG_OFFSET16 0x02

// ResTable_entry: size(2) flags(2), where size is the header's own length and the value follows it.
#define FF_ARSC_ENTRY_FLAG_COMPLEX 0x0001

// Res_value: size(2) res0(1) dataType(1) data(4). TYPE_STRING makes data an index into the global
// string pool; every other type is either a number or a reference to another resource.
#define FF_ARSC_OFF_VALUE_DATA_TYPE 3
#define FF_ARSC_OFF_VALUE_DATA 4
#define FF_ARSC_TYPE_STRING 0x03

// A type chunk's offset table has one word per entry, and this is what an entry with no value in that
// configuration looks like. It is not zero: offset zero is a real entry, at the start of the block.
#define FF_ARSC_NO_ENTRY 0xffffffffu

// The type a label resolves to. `string` is what `@string/...` compiles to, and it is what tells a
// label from the icons and banners that sit beside it in the same reply -- those point at `mipmap`
// or `drawable` entries whose value is a path inside the APK, so resolving them would hand back
// `res/9w.png` instead of a name.
#define FF_ANDROID_LABEL_STRING_TYPE "string"

static uint16_t readU16(const uint8_t* p) {
    return (uint16_t) ((uint16_t) p[0] | (uint16_t) ((uint16_t) p[1] << 8));
}

static uint32_t readU32(const uint8_t* p) {
    return (uint32_t) p[0] | ((uint32_t) p[1] << 8) | ((uint32_t) p[2] << 16) | ((uint32_t) p[3] << 24);
}

// One ResStringPool, of which a table has three.
typedef struct FFAndroidLabelPool {
    const uint8_t* table;
    size_t tableSize;
    size_t offset;
    uint32_t chunkSize;
    uint32_t count;
    uint32_t headerSize;
    uint32_t stringsStart;
    bool utf8;
} FFAndroidLabelPool;

[[gnu::nonnull(2), nodiscard]] static bool poolInit(FFAndroidLabelPool* pool, const uint8_t* table, size_t tableSize, size_t offset) {
    if (offset > tableSize || tableSize - offset < FF_ARSC_POOL_HEADER_SIZE) {
        return false;
    }
    if (readU16(table + offset) != FF_ARSC_STRING_POOL_TYPE) {
        return false;
    }
    pool->table = table;
    pool->tableSize = tableSize;
    pool->offset = offset;
    pool->chunkSize = readU32(table + offset + FF_ARSC_OFF_POOL_CHUNK_SIZE);
    pool->count = readU32(table + offset + FF_ARSC_OFF_POOL_COUNT);
    pool->headerSize = readU16(table + offset + 2);
    pool->stringsStart = readU32(table + offset + FF_ARSC_OFF_POOL_STRINGS_START);
    pool->utf8 = (readU32(table + offset + FF_ARSC_OFF_POOL_FLAGS) & FF_ARSC_POOL_FLAG_UTF8) != 0;
    return pool->headerSize >= FF_ARSC_POOL_HEADER_SIZE && pool->chunkSize > pool->headerSize;
}

// The length in front of a string. A UTF-8 pool writes one or two bytes, a UTF-16 pool one or two
// 16-bit words, and the top bit of the first says which of the two it is.
[[gnu::nonnull(1, 3), nodiscard]] static bool readLength8(const FFAndroidLabelPool* pool, size_t* position, uint32_t* length) {
    if (*position >= pool->tableSize) {
        return false;
    }
    uint32_t value = pool->table[(*position)++];
    if (value & 0x80) {
        if (*position >= pool->tableSize) {
            return false;
        }
        value = ((value & 0x7F) << 8) | pool->table[(*position)++];
    }
    *length = value;
    return true;
}

[[gnu::nonnull(1, 3), nodiscard]] static bool readLength16(const FFAndroidLabelPool* pool, size_t* position, uint32_t* length) {
    if (*position + 2 > pool->tableSize) {
        return false;
    }
    uint32_t value = readU16(pool->table + *position);
    *position += 2;
    if (value & 0x8000) {
        if (*position + 2 > pool->tableSize) {
            return false;
        }
        value = ((value & 0x7FFF) << 16) | readU16(pool->table + *position);
        *position += 2;
    }
    *length = value;
    return true;
}

[[gnu::nonnull(1)]] static void appendCodePoint(FFstrbuf* result, uint32_t code) {
    char encoded[4] = {};
    uint32_t length = 0;
    if (code < 0x80) {
        encoded[length++] = (char) code;
    } else if (code < 0x800) {
        encoded[length++] = (char) (0xC0 | (code >> 6));
        encoded[length++] = (char) (0x80 | (code & 0x3F));
    } else if (code < 0x10000) {
        encoded[length++] = (char) (0xE0 | (code >> 12));
        encoded[length++] = (char) (0x80 | ((code >> 6) & 0x3F));
        encoded[length++] = (char) (0x80 | (code & 0x3F));
    } else {
        encoded[length++] = (char) (0xF0 | (code >> 18));
        encoded[length++] = (char) (0x80 | ((code >> 12) & 0x3F));
        encoded[length++] = (char) (0x80 | ((code >> 6) & 0x3F));
        encoded[length++] = (char) (0x80 | (code & 0x3F));
    }
    ffStrbufAppendNS(result, length, encoded);
}

// One string out of a pool.
//
// The two lengths in front of a UTF-8 string are a *character* count and a *byte* count, and they
// differ for everything outside ASCII. Decoding `characters` bytes is the trap: a label of a few
// multi-byte characters comes back as its first character followed by replacement bytes. Measured on
// the test device, where every resource table read is a UTF-8 pool, this was the difference between
// the right label and mojibake.
[[gnu::nonnull(1, 3), nodiscard]] static bool poolGet(const FFAndroidLabelPool* pool, uint32_t index, FFstrbuf* result) {
    if (index >= pool->count) {
        return false;
    }
    const size_t offsets = pool->offset + pool->headerSize;
    if (offsets + ((size_t) index + 1) * 4 > pool->tableSize) {
        return false;
    }
    size_t position = pool->offset + pool->stringsStart + readU32(pool->table + offsets + (size_t) index * 4);
    if (position >= pool->tableSize) {
        return false;
    }

    if (pool->utf8) {
        uint32_t characters = 0;
        uint32_t bytes = 0;
        if (!readLength8(pool, &position, &characters) || !readLength8(pool, &position, &bytes)) {
            return false;
        }
        // An empty string is reported as a miss rather than written: the caller sets its fallback
        // before asking, and ffStrbufSetNS() with a length of zero *clears* the buffer, which would
        // throw that fallback away instead of leaving it alone.
        if (bytes == 0 || bytes > pool->tableSize - position) {
            return false;
        }
        ffStrbufSetNS(result, bytes, (const char*) (pool->table + position));
        return true;
    }

    uint32_t units = 0;
    if (!readLength16(pool, &position, &units)) {
        return false;
    }
    if (units == 0 || (size_t) units * 2 > pool->tableSize - position) {
        return false;
    }
    for (uint32_t i = 0; i < units; i++) {
        uint32_t code = readU16(pool->table + position + (size_t) i * 2);
        // A code point outside the basic plane arrives as a surrogate pair, and the two halves are
        // not characters of their own: half of a pair encoded on its own is not valid UTF-8.
        if (code >= 0xD800 && code <= 0xDBFF && i + 1 < units) {
            const uint32_t low = readU16(pool->table + position + ((size_t) i + 1) * 2);
            if (low >= 0xDC00 && low <= 0xDFFF) {
                code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                i++;
            }
        }
        appendCodePoint(result, code);
    }
    return true;
}

// The one ResTable_package this table defines, with the two pools a lookup starts from.
typedef struct FFAndroidLabelTable {
    const uint8_t* data;
    size_t size;
    FFAndroidLabelPool values;
    FFAndroidLabelPool types;
    size_t packageOffset;
    size_t packageHeaderSize;
    size_t packageSize;
    uint32_t packageId;
} FFAndroidLabelTable;

[[gnu::nonnull(1), nodiscard]] static bool tableInit(FFAndroidLabelTable* table, const uint8_t* data, size_t size) {
    // ResTable_header: header(8) packageCount(4). The header's own size is where the global pool
    // starts, which is 12 in every table measured -- but it is the field rather than the number.
    if (size < 12 || readU16(data) != FF_ARSC_TABLE_TYPE) {
        return false;
    }
    table->data = data;
    table->size = size;
    const uint32_t headerSize = readU16(data + 2);
    if (!poolInit(&table->values, data, size, headerSize)) {
        return false;
    }

    size_t position = (size_t) headerSize + table->values.chunkSize;
    while (position + 8 <= size) {
        const uint16_t type = readU16(data + position);
        const uint16_t chunkHeader = readU16(data + position + 2);
        const uint32_t chunkSize = readU32(data + position + 4);
        if (chunkSize < 8 || chunkSize > size - position) {
            break;
        }
        // A table can define several packages -- an app that links a shared library's resources has
        // more than one -- and the id in the resource reference is what picks the one to read.
        if (type == FF_ARSC_TABLE_PACKAGE_TYPE && chunkSize >= chunkHeader && chunkHeader >= FF_ARSC_OFF_PACKAGE_TYPE_STRINGS + 4) {
            table->packageOffset = position;
            table->packageHeaderSize = chunkHeader;
            table->packageSize = chunkSize;
            table->packageId = readU32(data + position + FF_ARSC_OFF_PACKAGE_ID);
            if (!poolInit(&table->types, data, size, position + readU32(data + position + FF_ARSC_OFF_PACKAGE_TYPE_STRINGS))) {
                return false;
            }
            return true;
        }
        position += chunkSize;
    }
    return false;
}

// A type chunk whose ResTable_config is all zeroes is the one that applies when nothing about the
// device or the locale matches a more specific chunk. An app with a translation has several chunks
// for one type, and picking the first one that happens along would answer with whatever language it
// was written in.
[[gnu::nonnull(1), nodiscard]] static bool isDefaultConfig(const uint8_t* config, size_t length) {
    if (length == 0) {
        return false;
    }
    for (size_t i = 0; i < length; i++) {
        if (config[i] != 0) {
            return false;
        }
    }
    return true;
}

[[gnu::nonnull(1, 5), nodiscard]] static bool resolveIn(const FFAndroidLabelTable* table, uint32_t typeId, uint32_t entryId, bool defaultOnly, FFstrbuf* result) {
    const size_t end = table->packageOffset + table->packageSize;
    for (size_t position = table->packageOffset + table->packageHeaderSize; position + 8 <= end;) {
        const uint16_t type = readU16(table->data + position);
        const uint16_t header = readU16(table->data + position + 2);
        const uint32_t size = readU32(table->data + position + 4);
        if (size < 8 || size > end - position) {
            return false;
        }
        if (type == FF_ARSC_TABLE_TYPE_TYPE && header >= FF_ARSC_OFF_TYPE_CONFIG) {
            const uint8_t id = table->data[position + FF_ARSC_OFF_TYPE_ID];
            const uint8_t flags = table->data[position + FF_ARSC_OFF_TYPE_FLAGS];
            if (id == typeId && (flags & (FF_ARSC_TYPE_FLAG_SPARSE | FF_ARSC_TYPE_FLAG_OFFSET16)) == 0 && size >= header) {
                const uint32_t entryCount = readU32(table->data + position + FF_ARSC_OFF_TYPE_ENTRY_COUNT);
                const uint32_t entriesStart = readU32(table->data + position + FF_ARSC_OFF_TYPE_ENTRIES_START);
                const size_t offsets = position + header;
                if (entryId < entryCount && offsets + ((size_t) entryId + 1) * 4 <= end) {
                    if (defaultOnly && !isDefaultConfig(table->data + position + FF_ARSC_OFF_TYPE_CONFIG, (size_t) header - FF_ARSC_OFF_TYPE_CONFIG)) {
                        position += size;
                        continue;
                    }
                    const uint32_t offset = readU32(table->data + offsets + (size_t) entryId * 4);
                    // `entriesStart` is relative to the chunk, not to the package, and both it and
                    // the entry's own offset come out of the file.
                    if (offset != FF_ARSC_NO_ENTRY && entriesStart <= size && offset <= size - entriesStart) {
                        const size_t entry = position + entriesStart + offset;
                        const size_t value = entry + readU16(table->data + entry);
                        if (value >= entry && value + 8 <= end && (readU16(table->data + entry + 2) & FF_ARSC_ENTRY_FLAG_COMPLEX) == 0
                            && table->data[value + FF_ARSC_OFF_VALUE_DATA_TYPE] == FF_ARSC_TYPE_STRING) {
                            return poolGet(&table->values, readU32(table->data + value + FF_ARSC_OFF_VALUE_DATA), result);
                        }
                    }
                }
            }
        }
        position += size;
    }
    return false;
}

// Resolves one resource id to its string, and only when the type it names is a `string`.
//
// The type name is the whole reason this reads the package's type pool rather than trusting the id:
// `labelRes` and the icon, logo and banner ids that sit beside it in the same reply are all resource
// ids of the same package, and the difference between them is the type they point at. A `mipmap` id
// resolves perfectly well -- to `res/9w.png`.
[[gnu::nonnull(1, 3), nodiscard]] static bool tableResolve(const FFAndroidLabelTable* table, uint32_t resourceId, FFstrbuf* result) {
    const uint32_t typeId = (resourceId >> 16) & 0xFF;
    if (((resourceId >> 24) & 0xFF) != table->packageId || typeId == 0) {
        return false;
    }

    FFstrbuf typeName;
    ffStrbufInit(&typeName);
    const bool isString = poolGet(&table->types, typeId - 1, &typeName) && ffStrbufEqualS(&typeName, FF_ANDROID_LABEL_STRING_TYPE);
    ffStrbufDestroy(&typeName);
    if (!isString) {
        return false;
    }

    // The configuration that applies is the unqualified one; a chunk that only applies to a locale
    // or a density is the fallback rather than the answer, which is what the second pass is for.
    return resolveIn(table, typeId, resourceId & 0xFFFF, true, result)
        || resolveIn(table, typeId, resourceId & 0xFFFF, false, result);
}

// ---------------------------------------------------------------------------------------------
// The reply
// ---------------------------------------------------------------------------------------------

// The reply's own copy of the APK path. `sourceDir` is a string in the ApplicationInfo parcel and it
// ends in `.apk`; nothing else in the reply does, which is what makes it findable without walking
// the parcel's field order -- that would mean recovering `ApplicationInfo.writeToParcel` out of the
// device's dex and then framing sixty fields of unknown width, to reach one string that is already
// recognizable.
[[gnu::nonnull(1, 3), nodiscard]] static bool findApkPath(const uint8_t* data, size_t size, char* result, size_t capacity) {
    for (size_t i = 8; i < size; i++) {
        if (data[i] != '/') {
            continue;
        }
        size_t end = i;
        while (end < size && data[end] >= 0x20 && data[end] < 0x7F) {
            end++;
        }
        const size_t length = end - i;
        if (length >= 5 && length < capacity && memcmp(data + end - 4, ".apk", 4) == 0) {
            memcpy(result, data + i, length);
            result[length] = '\0';
            return true;
        }
        i = end;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------

typedef struct FFAndroidLabelMapping {
    const uint8_t* data;
    size_t size;
} FFAndroidLabelMapping;

[[gnu::nonnull(1)]] static void labelUnmap(FFAndroidLabelMapping* mapping) {
    if (mapping->data != nullptr) {
        munmap((void*) mapping->data, mapping->size);
    }
}

[[gnu::nonnull(2), nodiscard]] static const char* labelMap(const char* path, FFAndroidLabelMapping* mapping) {
    const int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        FF_DEBUG("open(%s) failed: %s", path, strerror(errno));
        return "open(apk) failed";
    }

    struct stat status = {};
    const bool failed = fstat(fd, &status) != 0;
    if (failed || status.st_size <= 0) {
        FF_DEBUG("fstat(%s) reported %s", path, failed ? strerror(errno) : "an empty file");
        close(fd);
        return "fstat(apk) failed";
    }

    void* mapped = mmap(nullptr, (size_t) status.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (mapped == MAP_FAILED) {
        FF_DEBUG("mmap(%s, %lld bytes) failed: %s", path, (long long) status.st_size, strerror(errno));
        return "mmap(apk) failed";
    }
    mapping->data = mapped;
    mapping->size = (size_t) status.st_size;
    return nullptr;
}

// One reply buffer for every call this file makes, and it makes one per query.
static uint8_t s_reply[FF_ANDROID_LABEL_REPLY_SIZE];

const char* ffAndroidGetAppLabel(const char* packageName, FFstrbuf* result) {
    // The transaction code comes out of the jar rather than from a table here: it is a property of
    // the `.aidl` the device's ROM was built from, and a vendor that inserts a method ahead of this
    // one shifts every code behind it. See common/android/dex.h.
    int32_t transaction = FF_DEX_STATIC_INT_UNRESOLVED;
    const FFDexStaticIntRequest lookup = {
        .classDescriptor = FF_ANDROID_LABEL_STUB,
        .fieldName = FF_ANDROID_LABEL_TRANSACTION,
        .result = &transaction,
    };
    const char* error = ffDexStaticInts(FF_ANDROID_LABEL_JAR, &lookup, 1);
    if (error != nullptr) {
        return error;
    }
    if (transaction == FF_DEX_STATIC_INT_UNRESOLVED) {
        return "framework.jar does not declare IPackageManager.getApplicationInfo";
    }

    [[gnu::cleanup(ffBinderClose)]] FFBinder binder = { .fd = -1 };
    error = ffBinderOpen(&binder);
    if (error != nullptr) {
        return error;
    }

    [[gnu::cleanup(ffBinderServiceHandleRelease)]] FFBinderServiceHandle service = { .binder = &binder };
    error = ffBinderLookupService(&binder, FF_ANDROID_LABEL_SERVICE, FF_BINDER_SM_GET_SERVICE, &service.handle);
    if (error != nullptr) {
        FF_DEBUG("Looking up \"%s\" failed: %s", FF_ANDROID_LABEL_SERVICE, error);
        return error;
    }

    uint8_t request[FF_ANDROID_LABEL_REQUEST_SIZE];
    FFBinderParcel parcel = ffBinderParcelCreate(request, sizeof(request));
    ffBinderParcelPutInterfaceToken(&parcel, FF_ANDROID_LABEL_DESCRIPTOR);
    ffBinderParcelPutString16(&parcel, packageName);
    // `getApplicationInfo(String, long flags, int userId)`. The flags argument is 64-bit from Android
    // 13 on and 32-bit before it, and the wider one is what works on both: a release that reads an
    // int takes the low word and leaves the zero high word unread, because an AIDL stub stops
    // reading once its own arguments are in. The narrow one is not symmetric -- a release that reads
    // a long would take half of its own flags argument for the user id.
    ffBinderParcelPutU64(&parcel, 0);
    ffBinderParcelPutI32(&parcel, 0);

    FFBinderReply reply = ffBinderReplyCreate(s_reply, sizeof(s_reply));
    error = ffBinderTransact(&binder, service.handle, (uint32_t) transaction, 0, &parcel, &reply);
    if (error != nullptr) {
        return error;
    }
    if (ffBinderReplyIsStatus(&reply)) {
        return "The package manager rejected the request";
    }
    // The AIDL status first, then the presence marker of a nullable Parcelable: a package the caller
    // is not allowed to see answers an empty ApplicationInfo rather than an error.
    if (ffBinderReadI32(reply.data, reply.size, 0) != 0) {
        return "The package manager refused to describe the app";
    }
    if (ffBinderReadI32(reply.data, reply.size, 4) == 0) {
        FF_DEBUG("The package manager described \"%s\" as an empty ApplicationInfo", packageName);
        return nullptr;
    }

    char path[FF_ANDROID_LABEL_PATH_SIZE];
    if (!findApkPath(reply.data, reply.size, path, sizeof(path))) {
        return "The reply carries no APK path";
    }
    FF_DEBUG("\"%s\" is installed at \"%s\"", packageName, path);

    [[gnu::cleanup(labelUnmap)]] FFAndroidLabelMapping mapping = {};
    error = labelMap(path, &mapping);
    if (error != nullptr) {
        return error;
    }

    FFAndroidZipEntry table = {};
    error = ffAndroidZipFindEntry(mapping.data, mapping.size, FF_ANDROID_LABEL_TABLE, &table);
    if (error != nullptr) {
        return error;
    }
    if (table.method != FF_ZIP_METHOD_STORED) {
        FF_DEBUG("\"%s\" stores resources.arsc with compression method %u", packageName, (unsigned) table.method);
        return "The resource table is stored compressed";
    }

    FFAndroidLabelTable resources = {};
    if (!tableInit(&resources, table.payload, table.payloadSize)) {
        return "The resource table could not be read";
    }

    // Every word of the reply shaped like one of this package's resource ids, in the order they
    // appear, and the first one that resolves to a `string` is the label. See the header for what
    // that rule was audited against and why it beats recovering the parcel's field order.
    for (size_t offset = 8; offset + 4 <= reply.size; offset += 4) {
        if (tableResolve(&resources, readU32(reply.data + offset), result)) {
            FF_DEBUG("\"%s\" shows itself as \"%s\"", packageName, result->chars);
            return nullptr;
        }
    }
    FF_DEBUG("No resource id in the reply of \"%s\" resolves to a string", packageName);
    return nullptr;
}
