#include "displayserver.h"
#include "common/android/api.h"
#include "common/arrutil.h"
#include "common/settings.h"
#include "common/strutil.h"
#include "common/processing.h"
#include "linux/displayserver_linux.h"

#include <math.h>

static bool detectWithGetprop(FFDisplayServerResult* ds) {
    // Only for MiUI
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    if (ffSettingsGetAndroidProperty("persist.sys.miui_resolution", &buffer) &&
        ffStrbufContainC(&buffer, ',')) {
        // 1440,3200,560 => width,height,densityDpi
        uint32_t width = (uint32_t) ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrAfterFirstC(&buffer, ',');
        uint32_t height = (uint32_t) ffStrbufToUInt(&buffer, 0);
        ffStrbufSubstrAfterFirstC(&buffer, ',');
        uint32_t dpi = (uint32_t) ffStrbufToUInt(&buffer, 0) * 96 / 160;
        FFDisplayResult* display = ffdsAppendDisplay(ds,
            width,
            height,
            0,
            dpi,
            0,
            0,
            0,
            0,
            nullptr,
            FF_DISPLAY_TYPE_BUILTIN,
            false,
            0,
            0,
            0,
            "getprop");
        return !!display;
    }

    return false;
}

// `cmd display get-displays` and `dumpsys display` print the same thing -- the `DisplayInfo` of every
// display, one per line -- so one parser covers both and only the command and the marker in front of
// each record differ:
//
//  * `cmd display get-displays` needs no permission, which is what makes it usable for an app UID,
//    but the subcommand was only added to `DisplayManagerShellCommand` in Android 13. Android 11 and
//    12 answer `Unknown command: get-displays` on stdout with exit code 255.
//  * `dumpsys display` covers every release, including the ones that predate `get-displays`, but it
//    is gated behind `android.permission.DUMP`, so it only answers for `adb shell` and root.
//
// The record layout has changed across releases, and every difference is accepted rather than version
// checked:
//
//  * The mode list is printed as `modes [...]` up to Android 14 and as `supportedModes [...]` from
//    Android 15 on, which also prints `appsSupportedModes [...]` right behind it. Both spellings are
//    searched for.
//  * `renderFrameRate` is printed from Android 15 on. Before that the active mode's fps is the only
//    refresh rate the dump carries. The active mode's fps is preferred even where it exists, see
//    the comment on `refreshRate` below.
//  * `displayGroupId` is printed from Android 12 on, the physical dpi behind `density` from
//    Android 11 on, and `isForceSdr` from Android 15 on.
//
// A record is one line, and the `DisplayInfo{` inside it is what gets parsed, so the `Display id 0: `
// of the one command and the `mBaseDisplayInfo=` of the other are both skipped by the same code.
static bool detectWithCommand(FFDisplayServerResult* ds, char* const argv[], const char* marker, const char* platformApi) {
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
    FFProcessHandle handle;
    // `cmd` forwards its stdin to the service over binder, and the kernel rejects the whole
    // transaction when that fd is a terminal, which it is whenever fastfetch runs in a terminal.
    // Detaching the child from our stdin is only needed here, so the low level API is called instead
    // of `ffProcessAppendStdOut`.
    if (ffProcessSpawn(argv, false, ffGetNullFD(), &handle) != nullptr) {
        return false; // Neither command is available on every Android version
    }

    if (ffProcessReadOutput(&handle, &buf) != nullptr || buf.length == 0) {
        return false;
    }
    ffStrbufTrimRightSpace(&buf);

    uint32_t index = 0;
    while ((index = ffStrbufNextIndexS(&buf, index, marker)) < buf.length) {
        index += strlen(marker);

        uint32_t nextIndex = ffStrbufNextIndexC(&buf, index, '\n');
        buf.chars[nextIndex] = '\0';
        const char* info = buf.chars + index;

        // 0: DisplayInfo{"Builtin display", displayId 0, ..., real 1440 x 3168, ..., mode 2,
        //    renderFrameRate 60.000004, ..., defaultMode 4, ..., supportedModes [{id=2,
        //    width=1440, height=3168, fps=60.000004, ...}], ..., hdrCapabilities
        //    HdrCapabilities{mSupportedHdrTypes=[1, 2, 3, 4], ...}, isForceSdr false, ...,
        //    rotation 0, ..., type INTERNAL, uniqueId "local:4630946557703207059", ...,
        //    density 560 (560.0 x 560.0) dpi, ..., deviceProductInfo DeviceProductInfo{...,
        //    manufactureDate=ManufactureDate{week=27, year=2006}, ...}, ...}
        const char* field = strstr(info, "DisplayInfo{\"");
        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateA(64);
        if (field) {
            field += strlen("DisplayInfo{\"");
            const char* nameEnd = strchr(field, '"');
            if (nameEnd) {
                ffStrbufAppendNS(&name, (uint32_t) (nameEnd - field), field);
            }
        }

        // `real` is the size the display currently uses, which is smaller than the panel's when the
        // framework emulates a smaller display size
        unsigned width = 0, height = 0;
        if ((field = strstr(info, ", real ")) && sscanf(field, ", real %u x %u", &width, &height) < 2) {
            width = height = 0;
        }

        // `renderFrameRate` is printed from Android 15 on. It is documented as "a divisor of the
        // active mode refresh rate", so it is the rate the display is currently *rendering* at and
        // can be lower than the mode it is set to. It is therefore only used for a record whose
        // mode list can not be read, where a possibly divided rate still beats none.
        double renderFrameRate = 0;
        if ((field = strstr(info, ", renderFrameRate ")) && sscanf(field, ", renderFrameRate %lf", &renderFrameRate) < 1) {
            renderFrameRate = 0;
        }

        unsigned activeMode = 0, defaultMode = 0;
        if ((field = strstr(info, ", mode ")) && sscanf(field, ", mode %u", &activeMode) < 1) {
            activeMode = 0;
        }
        if ((field = strstr(info, ", defaultMode ")) && sscanf(field, ", defaultMode %u", &defaultMode) < 1) {
            defaultMode = 0;
        }

        // The modes are listed with their resolution in the natural orientation, which is also how
        // the preferred values are reported on the other platforms. `defaultMode` is the mode the
        // display itself prefers, so it carries the panel's native resolution.
        uint32_t preferredWidth = 0, preferredHeight = 0;
        double preferredRefreshRate = 0, activeModeRefreshRate = 0;
        field = strstr(info, ", supportedModes [");
        if (field == nullptr) {
            field = strstr(info, ", modes ["); // Android 14 and older
        }
        while (field && (field = strstr(field, "{id="))) {
            // {id=2, width=1440, height=3168, fps=60.000004, ...
            unsigned id = 0, modeWidth = 0, modeHeight = 0;
            double fps = 0;
            if (sscanf(field, "{id=%u, width=%u, height=%u, fps=%lf", &id, &modeWidth, &modeHeight, &fps) < 4) {
                break;
            }
            if (id == activeMode) {
                activeModeRefreshRate = fps;
            }
            if (id == defaultMode) {
                preferredWidth = modeWidth;
                preferredHeight = modeHeight;
                preferredRefreshRate = fps;
            }
            if (activeModeRefreshRate > 0 && preferredWidth > 0) {
                break; // Both are in, and the same list is printed a second time from Android 15 on
            }
            ++field;
        }

        // The nominal rate of the active mode, which is what `Display.getRefreshRate()` reports
        // (`refreshRateOverride` if it is set, the mode's own rate otherwise) and what every other
        // platform reports. `renderFrameRate` is deliberately not preferred: it is a render
        // cadence that follows the content rather than a property of the display, and it does not
        // exist before Android 15, so using it would make the reported rate change with the
        // Android version as well as with what is on screen.
        double refreshRate = activeModeRefreshRate;
        if (refreshRate <= 0) {
            refreshRate = renderFrameRate;
        }

        unsigned rotation = 0;
        if ((field = strstr(info, ", rotation ")) && sscanf(field, ", rotation %u", &rotation) < 1) {
            rotation = 0;
        }

        FFDisplayType type = FF_DISPLAY_TYPE_UNKNOWN;
        if ((field = strstr(info, ", type "))) {
            field += strlen(", type ");
            if (ffStrStartsWith(field, "INTERNAL")) {
                type = FF_DISPLAY_TYPE_BUILTIN;
            } else if (ffStrStartsWith(field, "EXTERNAL") || ffStrStartsWith(field, "WIFI")) {
                // A WIFI display is a wireless sink, which is as external as a wired one
                type = FF_DISPLAY_TYPE_EXTERNAL;
            }
        }

        unsigned density = 0;
        double physicalXDpi = 0, physicalYDpi = 0;
        if ((field = strstr(info, ", density "))) {
            // `density 640 (501.0411 x 509.28604) dpi`, the physical dpi is only printed since
            // Android 11
            if (sscanf(field, ", density %u (%lf x %lf) dpi", &density, &physicalXDpi, &physicalYDpi) < 1) {
                density = 0;
            }
        }

        // The physical dpi describes the panel itself and does not change with the logical display
        // size, so the physical size has to be derived from the native resolution
        uint32_t physicalWidth = 0, physicalHeight = 0;
        if (physicalXDpi > 0) {
            physicalWidth = (uint32_t) ((preferredWidth ? preferredWidth : width) * 25.4 / physicalXDpi + 0.5);
        }
        if (physicalYDpi > 0) {
            physicalHeight = (uint32_t) ((preferredHeight ? preferredHeight : height) * 25.4 / physicalYDpi + 0.5);
        }

        // `uniqueId` identifies the display across reboots, e.g. `local:4630946557703207059` on a
        // physical display and `virtual:...` on a virtual one
        uint64_t id = 0;
        if ((field = strstr(info, ", uniqueId \""))) {
            field += strlen(", uniqueId \"");
            const char* uniqueIdEnd = strchr(field, '"');
            const char* digits = uniqueIdEnd ? memchr(field, ':', (size_t) (uniqueIdEnd - field)) : nullptr;
            id = (uint64_t) strtoull(digits ? digits + 1 : field, nullptr, 10);
        }

        uint16_t manufactureYear = 0, manufactureWeek = 0;
        if ((field = strstr(info, ", deviceProductInfo "))) {
            // `manufactureDate=ManufactureDate{week=27, year=2006}`, either field may be `null`
            const char* date = strstr(field, "manufactureDate=ManufactureDate{");
            unsigned year = 0, week = 0;
            if (date && sscanf(date + strlen("manufactureDate=ManufactureDate{"), "week=%u, year=%u", &week, &year) == 2) {
                manufactureYear = (uint16_t) year;
                manufactureWeek = (uint16_t) week;
            } else if ((field = strstr(field, ", modelYear=")) && sscanf(field, ", modelYear=%u", &year) == 1) {
                // A display reports either the date of manufacture or the model year
                manufactureYear = (uint16_t) year;
            }
        }

        // `displayId` sits inside the record, not in front of it: `cmd` prints the id again in its
        // own prefix, but `dumpsys` prints only `mBaseDisplayInfo=`
        unsigned displayId = 0;
        if ((field = strstr(info, ", displayId ")) && sscanf(field, ", displayId %u", &displayId) < 1) {
            displayId = 0;
        }
        bool primary = displayId == 0; // Display 0 is the default one

        // Android counts density in dpi with 160 as the 1x baseline, fastfetch uses 96
        FFDisplayResult* display = ffdsAppendDisplay(ds,
            width,
            height,
            refreshRate,
            density * 96 / 160,
            preferredWidth,
            preferredHeight,
            preferredRefreshRate,
            rotation,
            &name,
            type,
            primary,
            id,
            physicalWidth,
            physicalHeight,
            platformApi);
        if (display) {
            display->manufactureYear = manufactureYear;
            display->manufactureWeek = manufactureWeek;

            // Reported for every display, not only for the built-in one: `hdrCapabilities` is a
            // field of the `DisplayInfo` record itself, so it describes that display and nothing
            // else, and the other platforms report HDR per display too (EDID on Linux, the
            // advanced color info per target on Windows). An external display or a wireless sink
            // carries the field as well.
            //
            // `hdrCapabilities HdrCapabilities{mSupportedHdrTypes=[1, 2, 3, 4], ...}` is printed
            // since Android 11, where an empty list means that the display can not do HDR at all.
            // The two fallbacks below it are device wide vendor properties, which is the price of
            // answering for a record that does not print the field.
            FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
            field = strstr(info, "hdrCapabilities HdrCapabilities{mSupportedHdrTypes=[");
            if (field) {
                field += strlen("hdrCapabilities HdrCapabilities{mSupportedHdrTypes=[");
                display->hdrStatus = *field == ']' ? FF_DISPLAY_HDR_STATUS_UNSUPPORTED : FF_DISPLAY_HDR_STATUS_SUPPORTED;
            } else if (ffSettingsGetAndroidProperty("ro.surface_flinger.has_HDR_display", &buffer)) {
                display->hdrStatus = ffStrbufIgnCaseEqualS(&buffer, "true") ? FF_DISPLAY_HDR_STATUS_SUPPORTED : FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
            } else {
                display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNKNOWN;
            }

            if (display->hdrStatus == FF_DISPLAY_HDR_STATUS_SUPPORTED) {
                // `persist.sys.hdr_mode` is non-zero while HDR is turned on, and `isForceSdr
                // true` means that the framework disabled every HDR capability of this display.
                // Note that `ffSettingsGetAndroidProperty` appends, so the value needs its own
                // buffer.
                FF_STRBUF_AUTO_DESTROY hdrMode = ffStrbufCreate();
                if (ffSettingsGetAndroidProperty("persist.sys.hdr_mode", &hdrMode) &&
                    ffStrbufToUInt(&hdrMode, 0) > 0 &&
                    !strstr(info, ", isForceSdr true")) {
                    display->hdrStatus = FF_DISPLAY_HDR_STATUS_ENABLED;
                }
            }
        }

        // The last display of the dump is not followed by a newline, so the loop must not step
        // past the end of the buffer (`ffStrbufNextIndexC` returns the length when it finds none)
        index = nextIndex < buf.length ? nextIndex + 1 : buf.length;
    }

    // A command that produced no `DisplayInfo` record has to be reported as a failure: the caller
    // keys off this value to decide whether to try the other command, and on Android 12 and older
    // `dumpsys` is the only one that can still yield a display.
    return ds->displays.length > 0;
}

static bool detectWithCmd(FFDisplayServerResult* ds) {
    return detectWithCommand(ds,
        (char*[]) { "/system/bin/cmd", "display", "get-displays", nullptr },
        "Display id ",
        "cmd");
}

static bool detectWithDumpsys(FFDisplayServerResult* ds) {
    // `dumpsys` needs android.permission.DUMP, which only the shell UID and root hold. Every other UID
    // is answered with `Permission Denial: can't dump DisplayManagerService from from pid=..., uid=...
    // due to missing android.permission.DUMP permission` on stdout and a zero exit status, so the fork
    // buys a child process and the record loop then finds no `DisplayInfo` -- the same "no display" the
    // caller reads as "try the next route". Not forking is the only difference this makes, but it is
    // the difference between the fallback chain describing what is available and it guessing.
    if (!ffAndroidIsRootOrShell(instance.state.platform.uid)) {
        return false;
    }

    return detectWithCommand(ds,
        (char*[]) { "/system/bin/dumpsys", "display", nullptr },
        "mBaseDisplayInfo=",
        "dumpsys");
}

// Several vendors embed the UI name and its version in `ro.build.display.id` without any
// separator, e.g. `MyOS12.0.14_A2121` or `RedMagicOS10.0.24_NX779J`.
static bool detectDEFromDisplayId(FFDisplayServerResult* ds, const char* const* names, uint32_t count) {
    FF_STRBUF_AUTO_DESTROY displayId = ffStrbufCreate();
    if (!ffSettingsGetAndroidProperty("ro.build.display.id", &displayId)) {
        return false;
    }

    for (uint32_t i = 0; i < count; i++) {
        uint32_t length = (uint32_t) strlen(names[i]);
        if (!ffStrbufStartsWithS(&displayId, names[i])) {
            continue;
        }

        ffStrbufSubstrBeforeFirstC(&displayId, '_'); // Drop the model suffix
        if (displayId.length > length) {
            ffStrbufInsertNC(&displayId, length, 1, ' ');
        }
        ffStrbufSet(&ds->dePrettyName, &displayId);
        return true;
    }

    return false;
}

static bool detectDE(FFDisplayServerResult* ds) {
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY brand = ffStrbufCreate();

    // `ffSettingsGetAndroidProperty` appends to the given buffer, so the brand has to be read
    // into its own buffer exactly once: reading it into a shared buffer repeatedly would
    // concatenate the values and break every comparison against it.
    ffSettingsGetAndroidProperty("ro.product.brand", &brand);

    // vivo reports the marketing name and version in `ro.vivo.os.build.display.id`,
    // separated by an underscore (`Funtouch OS_10`, `OriginOS 5`), and the build number
    // in `ro.vivo.product.version`.
    if (ffSettingsGetAndroidProperty("ro.vivo.os.build.display.id", &ds->dePrettyName)) {
        ffStrbufReplaceAllC(&ds->dePrettyName, '_', ' ');
        if (ffSettingsGetAndroidProperty("ro.vivo.product.version", &buffer)) {
            ffStrbufAppendC(&ds->dePrettyName, ' ');
            ffStrbufAppend(&ds->dePrettyName, &buffer);
        }
        return true;
    }

    // HarmonyOS 2.0 - 4.x is built on top of the Android framework and keeps reporting an
    // EMUI version, but it is HarmonyOS. `ro.build.ohos.devicetype` is set only by those
    // builds, and `hw_sc.build.platform.version` is the HarmonyOS version, while
    // `ro.build.version.emui` only carries the EMUI compatibility version
    // (HarmonyOS 2.0 == EMUI 12, 3.0 == 13, 4.0 == 14, 4.2 == 14.2, 4.3 == 15).
    if (ffSettingsGetAndroidProperty("ro.build.ohos.devicetype", &buffer)) {
        ffStrbufClear(&buffer);
        if (!ffSettingsGetAndroidProperty("hw_sc.build.platform.version", &buffer) &&
            ffSettingsGetAndroidProperty("ro.huawei.build.display.id", &buffer)) {
            // A few builds leave `hw_sc.build.platform.version` unset and only expose the
            // version through `ro.huawei.build.display.id`, e.g. `JKM-AL00 2.0.0.263(C00E260R4P3)`
            ffStrbufSubstrAfterFirstC(&buffer, ' ');
            ffStrbufSubstrBeforeFirstC(&buffer, '(');
            ffStrbufSubstrBeforeLastC(&buffer, '.');
        }
        if (buffer.length > 0) {
            ffStrbufSetF(&ds->dePrettyName, "HarmonyOS %s", buffer.chars);
        } else {
            ffStrbufSetStatic(&ds->dePrettyName, "HarmonyOS");
        }
        return true;
    }

    // HarmonyOS NEXT (5.0 and newer) drops the Android framework. It is the only Huawei
    // family reporting `ro.build.display.id` as `System 104.5.0.001(60J9)`, and none of its
    // properties carries the marketing version, so it can only be named without one.
    if (ffStrbufIgnCaseEqualS(&brand, "HUAWEI") &&
        ffSettingsGetAndroidProperty("ro.build.display.id", &buffer) &&
        ffStrbufStartsWithS(&buffer, "System ")) {
        ffStrbufSetStatic(&ds->dePrettyName, "HarmonyOS NEXT");
        return true;
    }

    // HONOR reports MagicOS / MagicUI in `ro.build.version.magic`. MagicUI 3.x stores a
    // bare version number (`3.0.1`) instead of a `MagicUI_x.y.z` string.
    if (ffSettingsGetAndroidProperty("ro.build.version.magic", &ds->dePrettyName)) {
        ffStrbufReplaceAllC(&ds->dePrettyName, '_', ' ');
        if (!ffStrbufStartsWithS(&ds->dePrettyName, "Magic")) {
            ffStrbufPrependS(&ds->dePrettyName, "MagicUI ");
        }
        return true;
    }

    if (ffSettingsGetAndroidProperty("ro.build.version.emui", &ds->dePrettyName)) {
        ffStrbufReplaceAllC(&ds->dePrettyName, '_', ' ');
        return true;
    }

    // Xiaomi HyperOS. `ro.mi.os.version.incremental` is `OS1.0.10.0.TLDCNXM`, while
    // `ro.build.version.incremental` still starts with `V816` on HyperOS 1.0, so it can
    // not be used to tell HyperOS and MIUI apart.
    if (ffSettingsGetAndroidProperty("ro.mi.os.version.name", &ds->dePrettyName)) {
        ffStrbufClear(&ds->dePrettyName);
        if (ffSettingsGetAndroidProperty("ro.mi.os.version.incremental", &ds->dePrettyName) &&
            ffStrbufStartsWithS(&ds->dePrettyName, "OS")) {
            ffStrbufSubstrAfter(&ds->dePrettyName, 1); // Drop the leading "OS"
            ffStrbufPrependS(&ds->dePrettyName, "HyperOS ");
        } else {
            ffStrbufSetStatic(&ds->dePrettyName, "HyperOS");
        }
        return true;
    }

    // Black Shark runs JOYUI, a MIUI fork, and therefore also sets
    // `ro.miui.ui.version.name`. `ro.build.version.incremental` is `V11.0.4.0.JOYUI`.
    if (ffStrbufIgnCaseEqualS(&brand, "blackshark")) {
        ffStrbufClear(&ds->dePrettyName);
        if (ffSettingsGetAndroidProperty("ro.build.version.incremental", &ds->dePrettyName)) {
            ffStrbufSubstrBeforeLastC(&ds->dePrettyName, '.'); // Drop the trailing "JOYUI"
            if (ffStrbufStartsWithS(&ds->dePrettyName, "V")) {
                ffStrbufSubstrAfter(&ds->dePrettyName, 0);
            }
            ffStrbufPrependS(&ds->dePrettyName, "JOYUI ");
        } else {
            ffStrbufSetStatic(&ds->dePrettyName, "JOYUI");
        }
        return true;
    }

    // MIUI. `ro.build.version.incremental` is `V14.0.1.0.TJJCNXM` on stable builds and a
    // bare release date (`21.11.17`) on beta builds.
    if (ffSettingsGetAndroidProperty("ro.miui.ui.version.name", &buffer)) {
        ffStrbufClear(&ds->dePrettyName);
        if (ffSettingsGetAndroidProperty("ro.build.version.incremental", &ds->dePrettyName)) {
            if (ffStrbufStartsWithS(&ds->dePrettyName, "V")) {
                ffStrbufSubstrAfter(&ds->dePrettyName, 0);
            }
            ffStrbufPrependS(&ds->dePrettyName, "MiUI ");
        } else {
            ffStrbufSetStatic(&ds->dePrettyName, "MiUI");
        }
        return true;
    }

    // realme UI is a ColorOS fork and reports both `ro.build.version.realmeui` and
    // `ro.build.version.oplusrom`; the former is the realme UI version.
    if (ffSettingsGetAndroidProperty("ro.build.version.realmeui", &ds->dePrettyName)) {
        if (ffStrbufStartsWithS(&ds->dePrettyName, "V")) {
            ffStrbufSubstrAfter(&ds->dePrettyName, 0);
        }
        ffStrbufPrependS(&ds->dePrettyName, "realme UI ");
        return true;
    }

    // ColorOS 12 and newer report `ro.build.version.oplusrom`, ColorOS 11 and older
    // report `ro.build.version.opporom`.
    if (ffSettingsGetAndroidProperty("ro.build.version.oplusrom", &ds->dePrettyName) ||
        ffSettingsGetAndroidProperty("ro.build.version.opporom", &ds->dePrettyName)) {
        if (ffStrbufStartsWithS(&ds->dePrettyName, "V")) {
            ffStrbufSubstrAfter(&ds->dePrettyName, 0);
        }
        ffStrbufPrependS(&ds->dePrettyName, "ColorOS ");
        return true;
    }

    if (ffSettingsGetAndroidProperty("ro.oxygen.version", &ds->dePrettyName)) {
        ffStrbufPrependS(&ds->dePrettyName, "OxygenOS ");
        return true;
    }

    // HydrogenOS is the Chinese counterpart of OxygenOS and reports `ro.rom.version`.
    if (ffStrbufEqualS(&brand, "OnePlus") &&
        ffSettingsGetAndroidProperty("ro.rom.version", &ds->dePrettyName)) {
        ffStrbufPrependS(&ds->dePrettyName, "H2OS ");
        return true;
    }

    if (ffSettingsGetAndroidProperty("ro.build.version.oneui", &ds->dePrettyName)) {
        // [ro.build.version.oneui]: [50101] => One UI 5.1.1
        // Samsung encodes the version with a base-100 carry: `major * 10000 + minor * 100 + patch`
        uint32_t version = (uint32_t) ffStrbufToUInt(&ds->dePrettyName, 0);
        uint32_t major = version / 10000;
        uint32_t minor = version / 100 % 100;
        uint32_t patch = version % 100;
        if (major == 0) {
            // Unexpected format, show the raw value
            ffStrbufPrependS(&ds->dePrettyName, "OneUI ");
        } else if (patch > 0) {
            ffStrbufSetF(&ds->dePrettyName, "OneUI %u.%u.%u", major, minor, patch);
        } else {
            ffStrbufSetF(&ds->dePrettyName, "OneUI %u.%u", major, minor);
        }
        return true;
    }

    // Flyme. `ro.build.display.id` is `Flyme 10.5.0.1A`, where the trailing `A` marks a
    // stable release. `ro.flyme.version.id` holds the same string, except on Flyme 12,
    // where it is an Android build id instead.
    if (ffStrbufIgnCaseEqualS(&brand, "meizu")) {
        if (!ffSettingsGetAndroidProperty("ro.build.display.id", &ds->dePrettyName) ||
            !ffStrbufStartsWithS(&ds->dePrettyName, "Flyme")) {
            ffStrbufSetStatic(&ds->dePrettyName, "Flyme");
        }
        return true;
    }

    // SmartisanOS. `ro.smartisan.version` is `4.2.6-201808311713-user-511` or
    // `6.6.6.2_TNT-201904101033-user-oce`.
    if (ffSettingsGetAndroidProperty("ro.smartisan.version", &ds->dePrettyName)) {
        ffStrbufSubstrBeforeFirstC(&ds->dePrettyName, '_');
        ffStrbufSubstrBeforeFirstC(&ds->dePrettyName, '-');
        ffStrbufPrependS(&ds->dePrettyName, "SmartisanOS ");
        return true;
    }

    // ZUI / ZUXOS. `ro.com.zui.version` is the internal ZUI version (`17.0` for
    // ZUXOS 1.1.10.138), while `ro.build.display.id` embeds the marketing name and
    // version: `TB321FU_CN_OPEN_USER_Q00011.0_V_ZUXOS_1.1.10.138_ST_250626`.
    if (ffSettingsGetAndroidProperty("ro.com.zui.version", &ds->dePrettyName)) {
        FF_STRBUF_AUTO_DESTROY displayId = ffStrbufCreate();
        const char* name = nullptr;
        if (ffSettingsGetAndroidProperty("ro.build.display.id", &displayId)) {
            if (ffStrbufSubstrAfterFirstS(&displayId, "_ZUXOS_")) {
                name = "ZUXOS";
            } else if (ffStrbufSubstrAfterFirstS(&displayId, "_ZUI_")) {
                name = "ZUI";
            }
            if (name) {
                ffStrbufSubstrBeforeFirstC(&displayId, '_'); // Drop the trailing "_ST_250626"
                ffStrbufSetF(&ds->dePrettyName, "%s %s", name, displayId.chars);
                return true;
            }
        }
        ffStrbufPrependS(&ds->dePrettyName, "ZUI "); // Moto builds only expose `ro.com.zui.version`
        return true;
    }

    // 360 OS (QiKU) reports `ro.build.uiversion` as `360UI:V3.0`.
    if (ffSettingsGetAndroidProperty("ro.build.uiversion", &ds->dePrettyName)) {
        uint32_t index = ffStrbufFirstIndexC(&ds->dePrettyName, ':');
        if (index < ds->dePrettyName.length) {
            // `360UI:V3.0` -> `360UI 3.0`
            ffStrbufRemoveSubstr(&ds->dePrettyName, index, index + 1);
            if (ds->dePrettyName.chars[index] == 'V') {
                ffStrbufRemoveSubstr(&ds->dePrettyName, index, index + 1);
            }
            ffStrbufInsertNC(&ds->dePrettyName, index, 1, ' ');
        }
        return true;
    }

    // LeEco EUI reports `ro.letv.release.version` as `6.0.030S`, where the trailing `S`
    // marks a stable release.
    if (ffSettingsGetAndroidProperty("ro.letv.release.version", &ds->dePrettyName)) {
        ffStrbufPrependS(&ds->dePrettyName, "EUI ");
        return true;
    }

    // nubia / ZTE
    if (ffStrbufIgnCaseEqualS(&brand, "nubia") || ffStrbufIgnCaseEqualS(&brand, "zte")) {
        // ObricUI reports `ro.os.ota.version`, e.g.
        // `1.8.0.2-20260204-125753-RELEASE-user-pacific-b911`
        if (ffSettingsGetAndroidProperty("ro.os.ota.version", &ds->dePrettyName)) {
            ffStrbufSubstrBeforeFirstC(&ds->dePrettyName, '-');
            ffStrbufPrependS(&ds->dePrettyName, "ObricUI ");
            return true;
        }

        // MyOS, NebulaAIOS and RedMagicOS embed the name and version in `ro.build.display.id`
        static const char* const displayIdNames[] = { "RedMagicOS", "NebulaAIOS", "MyOS" };
        if (detectDEFromDisplayId(ds, displayIdNames, ARRAY_SIZE(displayIdNames))) {
            return true;
        }

        // The older nubiaUI stores them in `ro.build.nubia.rom.name` and `.code`
        if (ffSettingsGetAndroidProperty("ro.build.nubia.rom.name", &ds->dePrettyName)) {
            if (ffSettingsGetAndroidProperty("ro.build.nubia.rom.code", &buffer)) {
                if (ffStrbufStartsWithS(&buffer, "V")) {
                    ffStrbufSubstrAfter(&buffer, 0); // `V1.0` -> `1.0`
                }
                ffStrbufAppendC(&ds->dePrettyName, ' ');
                ffStrbufAppend(&ds->dePrettyName, &buffer);
            }
            return true;
        }

        // MiFavor is the UI of the pre-MyOS ZTE phones; `ro.build.MiFavor_version` is a
        // bare version number (`4.0`). Note that MyOS, NebulaAIOS and RedMagicOS reuse
        // this property for their own version, so they are checked first.
        if (ffSettingsGetAndroidProperty("ro.build.MiFavor_version", &ds->dePrettyName)) {
            ffStrbufPrependS(&ds->dePrettyName, "MiFavor ");
            return true;
        }
    }

    // LineageOS and other AOSP-based distributions
    if (ffSettingsGetAndroidProperty("ro.lineage.build.version", &ds->dePrettyName)) {
        ffStrbufPrependS(&ds->dePrettyName, "LineageOS ");
        return true;
    }
    if (ffSettingsGetAndroidProperty("org.pixelexperience.version.display", &ds->dePrettyName)) {
        // PixelExperience_Plus_whyred-13.0-20230325-0421-OFFICIAL
        ffStrbufSubstrBeforeFirstC(&ds->dePrettyName, '-');
        ffStrbufSubstrBeforeLastC(&ds->dePrettyName, '_');
        ffStrbufReplaceAllC(&ds->dePrettyName, '_', ' ');
        return true;
    }

    if (ffStrbufEqualS(&brand, "asus") &&
        ffSettingsGetAndroidProperty("ro.build.version.incremental", &ds->dePrettyName)) {
        return true;
    }
    if (ffSettingsGetAndroidProperty("ro.build.display.id", &ds->dePrettyName)) {
        // Google Pixel and other devices running native Android
        return true;
    }

    return false;
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds) {
    // Keep the same display-server preference as Linux: Wayland provides the
    // richest output information, followed by XCB and XRandR.
    if (instance.config.general.dsForceDrm == FF_DS_FORCE_DRM_TYPE_FALSE) {
        ffdsConnectWayland(ds);

        if (ds->displays.length == 0) {
            ffdsConnectXcbRandr(ds);
        }

        if (ds->displays.length == 0) {
            ffdsConnectXrandr(ds);
        }

        if (ds->displays.length > 0) {
            ffdsDetectWMDE(ds);
            return;
        }
    }

    // https://source.android.com/docs/core/graphics/surfaceflinger-windowmanager
    ffStrbufSetStatic(&ds->wmProcessName, "system_server");
    ffStrbufSetStatic(&ds->wmPrettyName, "WindowManager"); // A system service managed by system_server
    ffStrbufSetStatic(&ds->wmProtocolName, FF_WM_PROTOCOL_SURFACEFLINGER);

    // `cmd` comes first because it needs no permission and therefore also works for an app UID.
    // `dumpsys` is the only route that answers on Android 12 and older, and only for `adb shell` and
    // root -- it is skipped without a fork for every other UID, see detectWithDumpsys -- and `getprop`
    // is MiUI specific and the last resort.
    if (!detectWithCmd(ds) && !detectWithDumpsys(ds)) {
        detectWithGetprop(ds);
    }

    detectDE(ds);
}
