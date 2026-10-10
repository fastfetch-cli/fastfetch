/*
 * Minimal hand-written replacement for the autoconf-generated config.h.
 *
 * Upstream libsixel ships config.h.in and lets `configure` produce config.h.
 * We only vendor the encoder-side subset (see README.md), and those files
 * reference a small, fixed set of macros, so the values are hard-coded here
 * instead of carrying the autotools machinery.
 *
 * The values below hold for every platform fastfetch targets:
 * Windows/MinGW-w64, Linux/glibc, Linux/musl, macOS and the BSDs.
 * Re-check this file whenever the vendored subset is re-synced with upstream.
 */

#ifndef LIBSIXEL_CONFIG_H
#define LIBSIXEL_CONFIG_H

/* Standard headers (upstream: AC_CHECK_HEADERS) */
#define HAVE_ASSERT_H 1
#define HAVE_ERRNO_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIMITS_H 1
#define HAVE_MATH_H 1
#define HAVE_MEMORY_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_TYPES_H 1

/* libc functions (upstream: AC_CHECK_FUNCS) */
#define HAVE_LDIV 1
#define HAVE_MALLOC 1
#define HAVE_REALLOC 1

/* Byte order of the 32bpp pixel readers in pixelformat.c; 0 = little endian */
#define SWAP_BYTES 0

/*
 * Guards the diagnostic pragma push/pop in dither.c. Not needed, and the
 * pragma spelling is not portable across the compilers we support.
 */
#define HAVE_DIAGNOSTIC_DEPRECATED_DECLARATIONS 0

/* Verbose quantizer tracing to stderr */
#define HAVE_DEBUG 0

/*
 * HAVE_TESTS must stay *undefined* rather than 0: sixel.h gates its test-only
 * declarations with `#ifdef HAVE_TESTS`, so defining it to 0 would still pull
 * in declarations for functions we do not compile.
 */
#undef HAVE_TESTS

/* Optional image loaders and features; not vendored */
#undef HAVE_GD
#undef HAVE_GDK_PIXBUF2
#undef HAVE_JPEG
#undef HAVE_LIBCURL
#undef HAVE_LIBPNG

#endif /* LIBSIXEL_CONFIG_H */
