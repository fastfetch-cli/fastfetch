#include "common/zlib.h"
#include "common/debug.h"

#if FF_HAVE_ZLIB && !FF_DISABLE_DLOPEN

struct FFZlibData zlibData;

// Opens the library and leaves the handle in zlibData, so the groups that follow resolve against the
// same mapping instead of opening their own.
static const char* openZlib(void) {
    #ifdef _WIN32
    FF_LIBRARY_LOAD_MESSAGE(zlib, "zlib1" FF_LIBRARY_EXTENSION, 2)
    #else
    FF_LIBRARY_LOAD_MESSAGE(zlib, "libz" FF_LIBRARY_EXTENSION, 2)
    #endif

    zlibData.handle = zlib;
    zlib = nullptr; // the handle is kept in zlibData, so the cleanup must not close it
    return nullptr;
}

const char* ffZlibLoad(FFZlibGroup group) {
    FF_DEBUG("Loading zlib group: %x", group);

    // Opened once, by whichever group is asked for first; the later ones reuse that mapping. A
    // failed open is retried by the next group rather than cached, so a system without libz pays at
    // most one attempt per group -- not one per call.
    if (zlibData.handle == nullptr) {
        if (zlibData.attempted == UINT8_MAX) {
            return "Previous attempt to open zlib failed";
        }

        const char* error = openZlib();
        if (error != nullptr) {
            static_assert(sizeof(zlibData.attempted) == 1, "zlibData.attempted must be 1 byte");
            zlibData.attempted = UINT8_MAX;
            return error;
        }
    }

    if (group & FF_ZLIB_COMPRESS) {
        if (zlibData.attempted & FF_ZLIB_COMPRESS) {
            if (!zlibData.ffcompress2) {
                return "Previous attempt to load compress failed";
            }
        } else {
            zlibData.attempted |= FF_ZLIB_COMPRESS;
            FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlibData.handle, zlibData, compressBound)
            FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlibData.handle, zlibData, compress2)
        }
    }
    if (group & FF_ZLIB_INFLATE) {
        if (zlibData.attempted & FF_ZLIB_INFLATE) {
            if (!zlibData.ffinflateEnd) {
                return "Previous attempt to load inflate failed";
            }
        } else {
            zlibData.attempted |= FF_ZLIB_INFLATE;
            FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlibData.handle, zlibData, inflateInit2_)
            FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlibData.handle, zlibData, inflate)
            FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlibData.handle, zlibData, inflateEnd)
        }
    }

    if (group & FF_ZLIB_GZFILE) {
        if (zlibData.attempted & FF_ZLIB_GZFILE) {
            if (!zlibData.ffgzclose) {
                return "Previous attempt to load gzfile failed";
            }
        } else {
            zlibData.attempted |= FF_ZLIB_GZFILE;
            FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlibData.handle, zlibData, gzopen)
            FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlibData.handle, zlibData, gzread)
            FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(zlibData.handle, zlibData, gzclose)
        }
    }

    return nullptr;
}

#endif
