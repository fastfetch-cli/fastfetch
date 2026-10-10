#include "common/android/package.h"
#include "common/android/api.h"
#include "common/debug.h"
#include "common/strutil.h"

#include <string.h>
#include <unistd.h>

// There is no way for a process to ask for its own package name: it is not in /proc/self/status, and
// an app cannot list /data/data. The executable path carries it instead -- an app's binaries live
// under /data/data/<package>/ or /data/user/<user>/<package>/, and /proc/self/exe resolves there.
// Under Termux that is /data/data/com.termux/files/usr/bin/<binary>, whose third component is the
// package.
//
// The memory cgroup carries the same information (4:memory:/apps/com.termux on the device this was
// verified on), but the unified hierarchy spells it out as /apps/uid_10478/pid_26943 with no name
// left in it, so the executable is the portable half of the two.
bool ffAndroidGetOwnPackage(char* buffer, size_t capacity) {
    char path[4096];
    const ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (length <= 0) {
        FF_DEBUG("Cannot read /proc/self/exe, so the calling package cannot be derived");
        return false;
    }
    path[length] = '\0';

    const char* rest = nullptr;
    const char* const dataPrefix = "/data/data/";
    const char* const userPrefix = "/data/user/";
    if (ffStrStartsWith(path, dataPrefix)) {
        rest = path + strlen(dataPrefix);
    } else if (ffStrStartsWith(path, userPrefix)) {
        // /data/user/<user>/<package>/... -- the user number has to be stepped over before the
        // package name starts, and only strchr() answers where that is.
        rest = strchr(path + strlen(userPrefix), '/');
        if (rest != nullptr) {
            rest += 1;
        }
    }

    if (rest == nullptr) {
        const uint32_t uid = instance.state.platform.uid;
        FF_DEBUG("The executable is not in an app data directory (\"%s\"), uid %u", path, uid);
        if (uid != FF_ANDROID_PRIVILEGED_UID_SHELL) {
            return false;
        }
        static const char shellPackage[] = FF_ANDROID_SHELL_PACKAGE;
        if (sizeof(shellPackage) > capacity) {
            return false;
        }
        memcpy(buffer, shellPackage, sizeof(shellPackage));
        FF_DEBUG("The calling package is \"%s\" (the shell owns it)", buffer);
        return true;
    }

    const char* end = strchr(rest, '/');
    const size_t nameLength = end != nullptr ? (size_t) (end - rest) : strlen(rest);
    if (nameLength == 0 || nameLength >= capacity) {
        return false;
    }
    memcpy(buffer, rest, nameLength);
    buffer[nameLength] = '\0';
    FF_DEBUG("The calling package is \"%s\"", buffer);
    return true;
}
