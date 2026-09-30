#include "wallpaper.h"
#include "common/android/binder.h"
#include "common/android/dex.h"
#include "common/android/package.h"
#include "common/debug.h"
#include "common/io.h"
#include "detection/displayserver/displayserver.h"

#include <unistd.h>

// Android keeps the wallpaper as a file per user, in a directory no app can enter:
//
//     /data/system/users/<userId>/wallpaper        the image the launcher draws
//     /data/system/users/<userId>/wallpaper_orig   the picture as it was uploaded, before cropping
//
// /data/system/users/<userId> is mode 0700 and owned by system:system, so `ls`, `stat` and `open` all
// answer EACCES to an app UID: the path is not readable even though it is not secret. What is
// reachable is a descriptor to the file, because android.app.IWallpaperManager opens it inside
// system_server and passes one back, and a descriptor is an ordinary file from then on. readlink(2)
// on it names the file, which is the string this module is asked for -- the image itself is never
// read or decoded.
//
// The gate is the calling package. `getWallpaper(String callingPackage, int which, int userId)`
// answers only when that name is the caller's own: the two-argument overload, which is the null name,
// comes back as a SecurityException raised by StorageManager.checkPermissionReadImages(), and an
// empty string, an unknown package and a real package belonging to a different uid all leave with no
// descriptor. That READ_MEDIA_IMAGES denial is not something a Termux-style app can lift -- the
// permission is not in its manifest and it targets SDK 28 -- so naming the caller is the only route.
//
// Measured on an Android 16 device as an app UID: the descriptor for `which` 1 is
// /data/system/users/0/wallpaper, a 2833949-byte JPEG whose SOF0 reads 2159x3168, while `which` 2
// answers with no descriptor at all because that lock screen is a live wallpaper.
//
// Only one of the two display servers is ever asked. The binder route is Android's, and it answers
// while SurfaceFlinger is the display server; a Termux:X11 or Wayland session puts a real compositor
// on top, and there the wallpaper belongs to that desktop -- so that case goes to wallpaper_linux.c,
// which is compiled alongside this file for exactly this reason.

#define FF_WALLPAPER_ANDROID_SERVICE "wallpaper"
#define FF_WALLPAPER_ANDROID_DESCRIPTOR "android.app.IWallpaperManager"

// WallpaperManager.FLAG_SYSTEM and FLAG_LOCK.
typedef enum FFWallpaperAndroidWhich : int32_t {
    FF_WALLPAPER_ANDROID_WHICH_SYSTEM = 1,
    FF_WALLPAPER_ANDROID_WHICH_LOCK = 2,
} FFWallpaperAndroidWhich;

// The jar and the stub class the transaction code is read out of. framework.jar answered on every
// device measured (Android 11, 13 and 16), so that is the one path and there is no fallback; the
// stub sits in classes.dex there, which is the entry the lookup tries first.
#define FF_WALLPAPER_ANDROID_JAR "/system/framework/framework.jar"
#define FF_WALLPAPER_ANDROID_STUB "Landroid/app/IWallpaperManager$Stub;"

// The method this module calls, by the name of the constant the dex carries for it. `getWallpaper`
// is the fourth method of IWallpaperManager$Stub, but the number is resolved from the device's own
// jar rather than written down here: the numbering is a property of the `.aidl` the ROM was built
// from, and a vendor that inserts a method ahead of this one shifts it, after which a transcribed
// code reaches a different method or none. Spelling the name out is the whole point -- the lookup
// turns it into whatever this build uses, and a build that does not declare it says so instead of
// reaching another method. See common/android/dex.h.
#define FF_WALLPAPER_ANDROID_GET_WALLPAPER "TRANSACTION_getWallpaper"

// `ffAndroidGetOwnPackage` writes at most this many bytes, terminator included.
#define FF_WALLPAPER_ANDROID_PACKAGE_SIZE 128

// The request is the interface token, the package name, `which` and `userId`. A string16 costs
// 4 + 2 * (length + 1) bytes padded to four, and the token puts three int32 in front of the
// descriptor. Deriving the size from the two names rather than rounding it up is what keeps the
// longest package name `ffAndroidGetOwnPackage` can produce from marking the parcel truncated --
// that fails the whole detection, rather than degrading to a call without a name.
#define FF_WALLPAPER_ANDROID_PARCEL_SIZE \
    (12 + ((4 + 2 * sizeof(FF_WALLPAPER_ANDROID_DESCRIPTOR) + 3) & ~3u) \
        + ((4 + 2 * FF_WALLPAPER_ANDROID_PACKAGE_SIZE + 3) & ~3u) + 8)

// The reply carries the descriptor object plus, for this method, a Bundle of two ints: 108 bytes on
// the device it was measured on, 80 when the wallpaper is a live one and the Bundle comes alone. Only
// the descriptor table is read out of it, so this is sized for headroom rather than for a field --
// being wrong here would fail the detection, not misread it.
#define FF_WALLPAPER_ANDROID_REPLY_SIZE 512

static const char* getWallpaperFile(FFBinder* binder, uint32_t handle, const char* package, FFWallpaperAndroidWhich which, int32_t transaction, FFstrbuf* result) {
    uint8_t parcelBuffer[FF_WALLPAPER_ANDROID_PARCEL_SIZE];
    FFBinderParcel parcel = ffBinderParcelCreate(parcelBuffer, sizeof(parcelBuffer));
    ffBinderParcelPutInterfaceToken(&parcel, FF_WALLPAPER_ANDROID_DESCRIPTOR);
    ffBinderParcelPutString16(&parcel, package);
    ffBinderParcelPutI32(&parcel, (int32_t) which);
    ffBinderParcelPutI32(&parcel, 0); // userId, which is the one user a phone has

    uint8_t replyBuffer[FF_WALLPAPER_ANDROID_REPLY_SIZE];
    FFBinderReply reply = ffBinderReplyCreate(replyBuffer, sizeof(replyBuffer));

    // TF_ACCEPT_FDS is neither optional nor a hint: a reply carrying a descriptor is only delivered
    // when the request asked for one, and a request that did not is answered with BR_FAILED_REPLY --
    // which reads like a service that is not running rather than like a missing flag.
    const char* error = ffBinderTransact(binder, handle, (uint32_t) transaction, TF_ACCEPT_FDS, &parcel, &reply);

    // Whatever else happened, a descriptor that did arrive is ours: the kernel installed it in our fd
    // table, and on the paths below that return early nothing else would close it.
    FF_AUTO_CLOSE_FD FFNativeFD fd = reply.fdCount > 0 ? reply.fds[0] : -1;

    if (error != nullptr) {
        FF_DEBUG("getWallpaper(which %d) could not be transacted: %s", (int) which, error);
        return error;
    }
    if (ffBinderReplyIsStatus(&reply)) {
        FF_DEBUG("getWallpaper(which %d) came back as a status reply of %d instead of a parcel",
            (int) which, (int) ffBinderReadI32(reply.data, reply.size, 0));
        return "Wallpaper service rejected the request";
    }

    const int32_t exception = ffBinderReadI32(reply.data, reply.size, 0);
    if (exception != 0) {
        // -1 is a SecurityException, and for this method it is what the package name check raises.
        FF_DEBUG("getWallpaper(which %d) raised exception %d over %zu bytes of reply", (int) which, exception, reply.size);
        return "Wallpaper service refused the request";
    }
    if (!ffIsValidNativeFD(fd)) {
        FF_DEBUG("getWallpaper(which %d) carried no descriptor, which is how a live wallpaper answers: "
                 "%zu bytes of reply",
            (int) which, reply.size);
        return "The wallpaper is not a file";
    }

    char path[64];
    const int pathLength = snprintf(path, sizeof(path), "/proc/self/fd/%d", (int) fd);
    if (pathLength <= 0 || (size_t) pathLength >= sizeof(path)) {
        return "Failed to build the /proc path of the wallpaper descriptor";
    }

    char target[512];
    const ssize_t length = readlink(path, target, sizeof(target) - 1);
    if (length <= 0) {
        FF_DEBUG("readlink(%s) failed", path);
        return "Failed to resolve the wallpaper path";
    }
    target[length] = '\0';

    ffStrbufSetNS(result, (uint32_t) length, target);
    FF_DEBUG("The wallpaper for which %d is \"%s\"", (int) which, result->chars);
    return nullptr;
}

const char* ffDetectWallpaper(FFstrbuf* result) {
    // The binder route below reads Android's own wallpaper, which is the one behind the launcher
    // only while SurfaceFlinger is the display server. A Termux:X11 or Wayland session runs a real
    // compositor on top of the device, and the wallpaper visible there belongs to that desktop, not
    // to Android -- so that case is handed to the Linux implementation, which knows GTK and Qt
    // settings. Wallpaper is only reported for one of the two, never merged.
    const FFDisplayServerResult* wm = ffConnectDisplayServer();
    if (!ffStrbufIgnCaseEqualS(&wm->wmProtocolName, FF_WM_PROTOCOL_SURFACEFLINGER)) {
        FF_DEBUG("The display server is \"%s\", so the Linux implementation answers", wm->wmProtocolName.chars);
        const char* ffDetectWallpaperLinux(FFstrbuf* result);
        return ffDetectWallpaperLinux(result);
    }

    char package[FF_WALLPAPER_ANDROID_PACKAGE_SIZE];
    if (!ffAndroidGetOwnPackage(package, sizeof(package))) {
        return "Cannot determine the package name of this process";
    }

    // Resolved before the binder is opened, so that a jar which does not declare the method says so
    // rather than the module failing later with a message about the transport. The reader reports a
    // code it cannot find by leaving the sentinel in the result and says nothing more -- what a
    // missing code means for this module is decided here, not there.
    int32_t transaction = FF_DEX_STATIC_INT_UNRESOLVED;
    const FFDexStaticIntRequest request = { FF_WALLPAPER_ANDROID_STUB, FF_WALLPAPER_ANDROID_GET_WALLPAPER, &transaction };
    const char* error = ffDexStaticInts(FF_WALLPAPER_ANDROID_JAR, &request, 1);
    if (error != nullptr) {
        FF_DEBUG("Reading \"%s\" from \"%s\" failed: %s", FF_WALLPAPER_ANDROID_GET_WALLPAPER, FF_WALLPAPER_ANDROID_JAR, error);
        return error;
    }
    if (transaction == FF_DEX_STATIC_INT_UNRESOLVED) {
        return "The wallpaper service does not declare getWallpaper";
    }

    [[gnu::cleanup(ffBinderClose)]] FFBinder binder = { .fd = -1 };
    error = ffBinderOpen(&binder);
    if (error != nullptr) {
        return error;
    }

    // Released on every path out of this function, including the ones below that return early.
    [[gnu::cleanup(ffBinderServiceHandleRelease)]] FFBinderServiceHandle service = { .binder = &binder };
    error = ffBinderLookupService(&binder, FF_WALLPAPER_ANDROID_SERVICE, FF_BINDER_SM_GET_SERVICE, &service.handle);
    if (error != nullptr) {
        FF_DEBUG("Looking up the \"%s\" service failed: %s", FF_WALLPAPER_ANDROID_SERVICE, error);
        return error;
    }
    FF_DEBUG("The \"%s\" service is handle %u, getWallpaper is transaction %d", FF_WALLPAPER_ANDROID_SERVICE, service.handle, transaction);

    // The system wallpaper is what the module means by "the current wallpaper"; the lock screen is a
    // second, separate picture, and is only reported when the system one is not a file -- which is
    // the live-wallpaper case, where a component draws it and there is no image behind it.
    error = getWallpaperFile(&binder, service.handle, package, FF_WALLPAPER_ANDROID_WHICH_SYSTEM, transaction, result);
    if (error == nullptr) {
        return nullptr;
    }

    FF_DEBUG("The system wallpaper is not a file (%s), so the lock screen is asked for instead", error);
    return getWallpaperFile(&binder, service.handle, package, FF_WALLPAPER_ANDROID_WHICH_LOCK, transaction, result);
}
