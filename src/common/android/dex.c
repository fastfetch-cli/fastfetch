#include "common/android/dex.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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

// The zip local file header, from APPNOTE.TXT 4.3.7. The central directory would carry the same
// fields, but the local header sits right in front of the data, so one walk over the file finds
// both the bounds and the payload.
#define FF_ZIP_LOCAL_HEADER_SIZE 30
#define FF_ZIP_OFF_METHOD 8
#define FF_ZIP_OFF_COMPRESSED_SIZE 18
#define FF_ZIP_OFF_UNCOMPRESSED_SIZE 22
#define FF_ZIP_OFF_NAME_LENGTH 26
#define FF_ZIP_OFF_EXTRA_LENGTH 28
#define FF_ZIP_METHOD_STORED 0
#define FF_ZIP_METHOD_DEFLATED 8

#define FF_DEX_ENTRY "classes.dex"
#define FF_DEX_MAGIC "dex\n"

static uint16_t dexU16(const uint8_t* p) {
    return (uint16_t) ((uint16_t) p[0] | (uint16_t) ((uint16_t) p[1] << 8));
}

static uint32_t dexU32(const uint8_t* p) {
    return (uint32_t) p[0] | ((uint32_t) p[1] << 8) | ((uint32_t) p[2] << 16) | ((uint32_t) p[3] << 24);
}

// LEB128, at most five bytes for the 32-bit values a dex stores this way.
static uint32_t dexUleb128(const uint8_t** p) {
    uint32_t value = 0;
    for (int shift = 0; shift <= 28; shift += 7) {
        const uint8_t byte = *(*p)++;
        value |= (uint32_t) (byte & 0x7f) << shift;
        if ((byte & 0x80) == 0) {
            break;
        }
    }
    return value;
}

// encoded_value: one header byte holding ((size - 1) << 5) | type, then that many payload bytes. A
// `static final int` constant is written as VALUE_INT, whose payload is a sign-extended
// little-endian integer. Reading through 64 bits keeps every shift in range even if the size field
// is nonsense; the caller's length check is what rejects such a dex.
static int32_t dexEncodedInt(const uint8_t** p) {
    const uint8_t header = *(*p)++;
    const uint32_t size = (uint32_t) (header >> 5) + 1;
    uint64_t value = 0;
    for (uint32_t i = 0; i < size && i < 8; ++i) {
        value |= (uint64_t) (*(*p)++) << (i * 8);
    }
    const uint32_t bits = size * 8;
    if (bits < 64 && (value & (1ull << (bits - 1))) != 0) {
        value |= ~((1ull << bits) - 1);
    }
    return (int32_t) (uint32_t) value;
}

// string_data_item: a uleb128 length in UTF-16 code units, then MUTF-8 bytes and a NUL terminator.
// The descriptors and field names looked up here are ASCII, where MUTF-8 and UTF-8 agree, so the
// bytes can be used as they are.
static const char* dexString(const uint8_t* dex, const uint8_t* end, uint32_t index) {
    const uint8_t* ids = dex + dexU32(dex + FF_DEX_OFF_STRING_IDS);
    if (ids < dex || (size_t) (end - ids) < (size_t) index * 4 + 4) {
        return nullptr;
    }
    const uint8_t* p = dex + dexU32(ids + (size_t) index * 4);
    if (p < dex || p >= end) {
        return nullptr;
    }
    (void) dexUleb128(&p);
    if (p >= end) {
        return nullptr;
    }
    return memchr(p, '\0', (size_t) (end - p)) != nullptr ? (const char*) p : nullptr;
}

// Walks the class's static field list and the class's static value array in step, and returns the
// value paired with `fieldName`. Both lists are ordered by field index and cover exactly the same
// fields -- when they do not, the pairing is not trustworthy and nothing is returned.
static const char* dexStaticInt(const uint8_t* dex, size_t size, const char* classDescriptor, const char* fieldName, int32_t* result) {
    const uint8_t* end = dex + size;

    if (size < FF_DEX_HEADER_SIZE || dexU32(dex + FF_DEX_OFF_ENDIAN_TAG) != FF_DEX_ENDIAN_TAG) {
        return "Not a dex file";
    }
    const uint32_t fileSize = dexU32(dex + FF_DEX_OFF_FILE_SIZE);
    if (fileSize > size || fileSize < FF_DEX_HEADER_SIZE) {
        return "The dex file size is out of range";
    }
    end = dex + fileSize;

    // The class's type index, so that the class_def_item and the field ids can be filtered by class.
    const uint8_t* types = dex + dexU32(dex + FF_DEX_OFF_TYPE_IDS);
    const uint32_t typeCount = dexU32(dex + FF_DEX_OFF_TYPE_IDS_SIZE);
    if (types < dex || (size_t) (end - types) < (size_t) typeCount * 4) {
        return "The dex type table is out of range";
    }
    uint32_t typeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < typeCount; ++i) {
        const char* descriptor = dexString(dex, end, dexU32(types + (size_t) i * 4));
        if (descriptor != nullptr && strcmp(descriptor, classDescriptor) == 0) {
            typeIndex = i;
            break;
        }
    }
    if (typeIndex == UINT32_MAX) {
        return "The class is not in the dex";
    }

    const uint8_t* classes = dex + dexU32(dex + FF_DEX_OFF_CLASS_DEFS);
    const uint32_t classCount = dexU32(dex + FF_DEX_OFF_CLASS_DEFS_SIZE);
    if (classes < dex || (size_t) (end - classes) < (size_t) classCount * FF_DEX_CLASS_DEF_SIZE) {
        return "The dex class table is out of range";
    }

    for (uint32_t i = 0; i < classCount; ++i) {
        const uint8_t* classDef = classes + (size_t) i * FF_DEX_CLASS_DEF_SIZE;
        if (dexU32(classDef) != typeIndex) {
            continue;
        }

        const uint8_t* fields = dex + dexU32(classDef + FF_DEX_OFF_CLASS_DEF_DATA);
        const uint8_t* values = dex + dexU32(classDef + FF_DEX_OFF_CLASS_DEF_STATIC_VALUES);
        if (fields < dex || fields >= end || values < dex || values >= end) {
            return "The dex class data is out of range";
        }

        const uint32_t staticFields = dexUleb128(&fields);
        (void) dexUleb128(&fields); // instance_fields_size
        (void) dexUleb128(&fields); // direct_methods_size
        (void) dexUleb128(&fields); // virtual_methods_size
        const uint32_t staticValues = dexUleb128(&values);
        if (staticFields != staticValues) {
            return "The dex static fields and values do not pair up";
        }

        const uint8_t* fieldIds = dex + dexU32(dex + FF_DEX_OFF_FIELD_IDS);
        uint32_t fieldIndex = 0;
        for (uint32_t j = 0; j < staticFields; ++j) {
            if (fields >= end) {
                return "The dex static field list is truncated";
            }
            fieldIndex += dexUleb128(&fields); // field_idx_diff
            (void) dexUleb128(&fields);        // access_flags
            const int32_t value = dexEncodedInt(&values);

            if ((size_t) (end - fieldIds) < ((size_t) fieldIndex + 1) * FF_DEX_FIELD_ID_SIZE) {
                return "The dex field table is out of range";
            }
            const char* name = dexString(dex, end, dexU32(fieldIds + (size_t) fieldIndex * FF_DEX_FIELD_ID_SIZE + FF_DEX_OFF_FIELD_ID_NAME));
            if (name != nullptr && strcmp(name, fieldName) == 0) {
                *result = value;
                return nullptr;
            }
        }
        return "The field is not a static field of the class";
    }
    return "The class has no class_def_item";
}

// ---------------------------------------------------------------------------------------------
// Locating classes.dex
// ---------------------------------------------------------------------------------------------

typedef struct FFDexMapping {
    uint8_t* mapped;      // the jar, as mapped; nullptr once released
    size_t mappedSize;
    const uint8_t* data;  // the dex bytes: inside `mapped`, or `inflated`
    size_t size;
    uint8_t* inflated;    // owned; only set when the entry had to be decompressed
} FFDexMapping;

static void wrapDexMapping(FFDexMapping* mapping) {
    assert(mapping);
    free(mapping->inflated);
    if (mapping->mapped != nullptr) {
        munmap(mapping->mapped, mapping->mappedSize);
    }
}

// The local file header of `classes.dex` carries both the payload bounds and its compression, so a
// single walk over the jar is enough to find it either way. AOSP builds framework jars with the
// entry STORED so that ART can map it, which is the common case and needs no decompression.
static const char* findDexEntry(const uint8_t* jar, size_t jarSize, const uint8_t** data, size_t* dataSize, uint32_t* uncompressedSize, uint16_t* method) {
    for (size_t i = 0; i + FF_ZIP_LOCAL_HEADER_SIZE <= jarSize; ++i) {
        if (memcmp(jar + i, "PK\x03\x04", 4) != 0) {
            continue;
        }
        const uint16_t nameLength = dexU16(jar + i + FF_ZIP_OFF_NAME_LENGTH);
        const uint16_t extraLength = dexU16(jar + i + FF_ZIP_OFF_EXTRA_LENGTH);
        if (nameLength != sizeof(FF_DEX_ENTRY) - 1) {
            continue;
        }
        const size_t headerSize = FF_ZIP_LOCAL_HEADER_SIZE + (size_t) nameLength + (size_t) extraLength;
        if (headerSize > jarSize - i) {
            continue;
        }
        if (memcmp(jar + i + FF_ZIP_LOCAL_HEADER_SIZE, FF_DEX_ENTRY, sizeof(FF_DEX_ENTRY) - 1) != 0) {
            continue;
        }

        const uint32_t compressedSize = dexU32(jar + i + FF_ZIP_OFF_COMPRESSED_SIZE);
        if (compressedSize == 0 || compressedSize > jarSize - i - headerSize) {
            // A zero size means a data descriptor follows the payload instead of the header; the
            // magic scan in the caller still covers that case as long as the entry is STORED.
            continue;
        }
        *method = dexU16(jar + i + FF_ZIP_OFF_METHOD);
        *uncompressedSize = dexU32(jar + i + FF_ZIP_OFF_UNCOMPRESSED_SIZE);
        *data = jar + i + headerSize;
        *dataSize = compressedSize;
        return nullptr;
    }
    return "The jar has no usable classes.dex entry";
}

// Fallback for a jar whose entry is STORED but whose header has no sizes: the dex magic is then a
// literal run of bytes in the file, and the header that follows it validates or it does not.
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

    uint8_t* buffer = malloc(uncompressedSize);
    if (buffer == nullptr) {
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

    if (ffinflateInit2_(&stream, -MAX_WBITS, ZLIB_VERSION, (int) sizeof(z_stream)) != Z_OK) {
        free(buffer);
        return "inflateInit2 failed";
    }
    const int status = ffinflate(&stream, Z_FINISH);
    ffinflateEnd(&stream);
    if (status != Z_STREAM_END || stream.total_out != (uLong) uncompressedSize) {
        free(buffer);
        return "Inflating classes.dex failed";
    }

    *out = buffer;
    return nullptr;
}
#endif

const char* ffDexStaticInt(const char* jarPath, const char* classDescriptor, const char* fieldName, int32_t* result) {
    [[gnu::cleanup(wrapDexMapping)]] FFDexMapping mapping = {};

    const int fd = open(jarPath, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return "open(jar) failed";
    }

    struct stat st = {};
    if (fstat(fd, &st) != 0 || st.st_size <= 0) {
        close(fd);
        return "fstat(jar) failed";
    }
    mapping.mappedSize = (size_t) st.st_size;
    mapping.mapped = mmap(nullptr, mapping.mappedSize, PROT_READ, MAP_PRIVATE, fd, 0);
    // The mapping outlives the descriptor, so the descriptor can go either way from here.
    close(fd);
    if (mapping.mapped == MAP_FAILED) {
        mapping.mapped = nullptr;
        return "mmap(jar) failed";
    }

    const uint8_t* data = nullptr;
    size_t dataSize = 0;
    uint32_t uncompressedSize = 0;
    uint16_t method = FF_ZIP_METHOD_STORED;

    if (findDexEntry(mapping.mapped, mapping.mappedSize, &data, &dataSize, &uncompressedSize, &method) != nullptr) {
        data = findDexMagic(mapping.mapped, mapping.mappedSize, &dataSize);
        if (data == nullptr) {
            return "No dex in the jar";
        }
    } else if (method == FF_ZIP_METHOD_STORED) {
        dataSize = uncompressedSize;
    } else if (method == FF_ZIP_METHOD_DEFLATED) {
        #ifdef FF_HAVE_ZLIB
        const char* error = inflateDex(data, dataSize, uncompressedSize, &mapping.inflated);
        if (error != nullptr) {
            return error;
        }
        data = mapping.inflated;
        dataSize = uncompressedSize;
        #else
        return "The jar deflates classes.dex and fastfetch was built without zlib";
        #endif
    } else {
        return "classes.dex uses an unsupported compression method";
    }

    mapping.data = data;
    mapping.size = dataSize;
    return dexStaticInt(mapping.data, mapping.size, classDescriptor, fieldName, result);
}
