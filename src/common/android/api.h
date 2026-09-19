#pragma once

// The NDK marks an API unavailable whenever it is newer than the API level the build targets, and
// fastfetch keeps running on devices below the levels it uses: AImageDecoder is 30, decoding past
// the first frame is 31, and AMediaCodec_getName is 28. CMake compiles the files that reach for
// them with -D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__ -Werror=unguarded-availability, which turns
// those entry points into weak references and makes every unguarded use an error. Two things follow:
//
//   * A weak reference resolves to null on an older device, so every use needs a run-time check --
//     that is FF_API_AT_LEAST. Unguarded, the call would jump to a null pointer.
//   * Annotating a helper with FF_REQUIRES_API keeps the check in one place: the compiler rejects
//     any call of that helper which is not itself inside a guard.
//
// https://developer.android.com/ndk/guides/using-newer-apis
#define FF_REQUIRES_API(x) [[clang::availability(android, introduced = x)]]
#define FF_API_AT_LEAST(x) __builtin_available(android x, *)
