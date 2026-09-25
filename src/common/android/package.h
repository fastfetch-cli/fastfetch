#pragma once

#include <stdbool.h>
#include <stddef.h>

// The one package name the shell UID owns. android.os.Process.SHELL_UID is a single app UID that
// owns exactly one package, so a process that cannot derive a package of its own -- a static build
// pushed to /data/local/tmp is the case that showed up -- can answer with this name and have the
// system services accept it. See FF_ANDROID_PRIVILEGED_UID_SHELL in common/android/api.h.
#define FF_ANDROID_SHELL_PACKAGE "com.android.shell"

// Some of the system services take the caller's package name as an argument and check it against the
// calling UID rather than against the name being real: a name nobody owns, and a real name belonging
// to another app, are refused exactly like a null one. So it has to be the caller's own name.
//
// Whether the name is checked at all depends on the service and the release -- `getConnectionInfo`
// answered "Package com.termux does not belong to <shell uid>" on one Android 16 device while the
// same name was accepted on an Android 11 one, and `getWallpaper` refuses a null, empty or foreign
// name on that Android 16 device -- so passing the real one is what works everywhere.
//
// Writes the name into `buffer`, terminator included, and returns whether one was found. `capacity`
// is the caller's, because the parcel the name ends up in is sized from it.
[[gnu::nonnull(1), nodiscard]] bool ffAndroidGetOwnPackage(char* buffer, size_t capacity);
