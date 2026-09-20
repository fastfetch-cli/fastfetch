#pragma once

#include <stdbool.h>
#include <stdint.h>

// The NDK marks an API unavailable whenever it is newer than the API level the build targets, and
// fastfetch keeps running on devices below the levels it uses: AImageDecoder is 30, decoding past
// the first frame is 31, and AMediaCodec_getName is 28. CMake compiles the files that reach for
// them with -D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__ -Werror=unguarded-availability, which turns
// those entry points into weak references and makes every unguarded use an error. Two things follow:
//
//   * A weak reference resolves to null on an older device, so every use needs a run-time check --
//     that is FF_ANDROID_API_AT_LEAST. Unguarded, the call would jump to a null pointer.
//   * Annotating a helper with FF_ANDROID_REQUIRES_API keeps the check in one place: the compiler rejects
//     any call of that helper which is not itself inside a guard.
//
// https://developer.android.com/ndk/guides/using-newer-apis
#define FF_ANDROID_REQUIRES_API(x) [[clang::availability(android, introduced = x)]]
#define FF_ANDROID_API_AT_LEAST(x) __builtin_available(android x, *)

// The UIDs that hold android.permission.DUMP, which is what `dumpsys` checks before it prints
// anything: every other UID is refused on stdout and the command still exits 0, which a caller cannot
// tell apart from a command that printed nothing useful. Forking a `dumpsys` from anywhere else is
// therefore pure waste, and for some fields `dumpsys` is the only route there is.
//
// The refusal is not always the permission, and the text differs per service and per release. On
// an Android 16 device an app UID gets `Permission Denial: can't dump DisplayManagerService ... due to
// missing android.permission.DUMP permission` for `display`, but `Can't find service: battery` for
// `battery` -- that service is not in the list `dumpsys -l` shows an app at all, while `display` is.
// The UID is what decides, so one check covers both.
//
// android.os.Process.ROOT_UID and SHELL_UID. The shell UID has a second use: it owns exactly one
// package name, which the system services accept, so a process that cannot derive its own package
// can answer with that one instead.
typedef enum FFAndroidPrivilegedUid : uint32_t {
    FF_ANDROID_PRIVILEGED_UID_ROOT = 0,
    FF_ANDROID_PRIVILEGED_UID_SHELL = 2000,
} FFAndroidPrivilegedUid;

// Whether the given UID is allowed to run `dumpsys`. The UID is passed in rather than read here, so
// that the caller can take it from `instance.state.platform.uid` -- which is what makes the
// privileged path reachable from a test that does not run as root or as the shell.
static inline bool ffAndroidIsRootOrShell(uint32_t uid) {
    return uid == FF_ANDROID_PRIVILEGED_UID_ROOT || uid == FF_ANDROID_PRIVILEGED_UID_SHELL;
}
