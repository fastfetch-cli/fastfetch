#pragma once

#include "fastfetch.h"

#if FF_HAVE_ZLIB

// Which set of entry points a caller needs. Resolving is per group rather than all at once so that a
// symbol nobody uses cannot make a caller fail: libz is not one library across the platforms
// fastfetch builds on, and the gz family in particular is not guaranteed to be present.
typedef enum FFZlibGroup : uint8_t {
    FF_ZLIB_COMPRESS = 1 << 0,    // compressBound, compress2
    FF_ZLIB_INFLATE = 1 << 1,     // inflateInit2_, inflate, inflateEnd
    FF_ZLIB_GZFILE = 1 << 2,      // gzopen, gzread, gzclose
} FFZlibGroup;

    #if FF_DISABLE_DLOPEN

        #include <zlib.h>

static inline const char* ffZlibLoad([[maybe_unused]] FFZlibGroup group) {
    // the library is linked in: nothing to open, nothing to resolve, nothing to fail
    return nullptr;
}

    #else
        // Hack. <zlib.h> declares everything with an extern linkage macro, which prevents us from
        // implementing the functions ourselves. zlib spells it ZEXTERN/ZEXPORT and zlib-ng's
        // zlib-compat header spells it Z_EXTERN/Z_EXPORT; both set theirs under `#ifndef`, so
        // defining all four before the include is what makes the definitions at the bottom of this
        // header legal -- they inherit the static inline linkage from the declarations zlib.h emits
        // here. This header must therefore be the first thing in a translation unit to pull in
        // <zlib.h>; if it is not, the definitions below stop being static and clang reports
        // `static declaration follows non-static declaration` right there.
        #undef ZEXTERN
        #undef ZEXPORT
        #undef Z_EXTERN
        #undef Z_EXPORT
        #define ZEXTERN static inline
        #define ZEXPORT
        #define Z_EXTERN static inline
        #define Z_EXPORT

        // Redeclaring everything as `static inline` above means <zlib.h> now defines entry points
        // that a given translation unit may not call at all. Silenced for the include only: the
        // pragma is popped right after, so the rest of the file -- and the rest of the translation
        // unit that includes this header -- still reports unused functions.
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wunused-function"

        #include <zlib.h>

        #pragma GCC diagnostic pop

        #include "common/library.h"

extern struct FFZlibData {
    // Kept open rather than closed once opened: every group resolves against this one mapping, and
    // re-opening per group would pay the cold load again.
    void* handle;

    FF_LIBRARY_SYMBOL(compressBound)
    FF_LIBRARY_SYMBOL(compress2)
    FF_LIBRARY_SYMBOL(inflateInit2_)
    FF_LIBRARY_SYMBOL(inflate)
    FF_LIBRARY_SYMBOL(inflateEnd)
    FF_LIBRARY_SYMBOL(gzopen)
    FF_LIBRARY_SYMBOL(gzread)
    FF_LIBRARY_SYMBOL(gzclose)

    FFZlibGroup attempted;
} zlibData;

[[gnu::always_inline]] static inline uLong(compressBound)(uLong sourceLen) {
    return zlibData.ffcompressBound(sourceLen);
}

[[gnu::always_inline]] static inline int(compress2)(Bytef* dest, uLongf* destLen, const Bytef* source, uLong sourceLen, int level) {
    return zlibData.ffcompress2(dest, destLen, source, sourceLen, level);
}

[[gnu::always_inline]] static inline int(inflateInit2_)(z_streamp strm, int windowBits, const char* version, int stream_size) {
    return zlibData.ffinflateInit2_(strm, windowBits, version, stream_size);
}

[[gnu::always_inline]] static inline int(inflate)(z_streamp strm, int flush) {
    return zlibData.ffinflate(strm, flush);
}

[[gnu::always_inline]] static inline int(inflateEnd)(z_streamp strm) {
    return zlibData.ffinflateEnd(strm);
}

[[gnu::always_inline]] static inline gzFile(gzopen)(const char* path, const char* mode) {
    return zlibData.ffgzopen(path, mode);
}

[[gnu::always_inline]] static inline int(gzread)(gzFile file, voidp buf, unsigned len) {
    return zlibData.ffgzread(file, buf, len);
}

[[gnu::always_inline]] static inline int(gzclose)(gzFile file) {
    return zlibData.ffgzclose(file);
}

// Resolves the entry points of `group` and returns nullptr, or an error message. Call it before using
// that group's functions: until it has succeeded, the forwarders above are null pointers. The library
// itself is opened once, by whichever group is asked for first, and stays open.
const char* ffZlibLoad(FFZlibGroup group);

    #endif

#endif
