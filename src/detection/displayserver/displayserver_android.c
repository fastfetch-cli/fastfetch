#include "displayserver.h"
#include "common/arrutil.h"
#include "common/settings.h"
#include "common/processing.h"
#include "linux/displayserver_linux.h"

#include <math.h>

static bool checkHdrStatus(FFDisplayResult* display) {
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    if (ffSettingsGetAndroidProperty("ro.surface_flinger.has_HDR_display", &buffer)) {
        if (ffStrbufIgnCaseEqualS(&buffer, "true")) {
            display->hdrStatus = FF_DISPLAY_HDR_STATUS_SUPPORTED;

            if (ffSettingsGetAndroidProperty("persist.sys.hdr_mode", &buffer) &&
                ffStrbufToUInt(&buffer, 0) > 0) {
                display->hdrStatus = FF_DISPLAY_HDR_STATUS_ENABLED;
            }

            return true;
        } else {
            display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
            return true;
        }
    }

    display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNKNOWN;
    return false;
}

static void detectWithDumpsys(FFDisplayServerResult* ds) {
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buf, (char*[]) {
                                        "/system/bin/dumpsys",
                                        "display",
                                        nullptr,
                                    }) != nullptr ||
        buf.length == 0) {
        return; // Only works in `adb shell`, or when rooted
    }

    uint32_t index = 0;
    while ((index = ffStrbufNextIndexS(&buf, index, "DisplayDeviceInfo")) < buf.length) {
        index += strlen("DisplayDeviceInfo");
        uint32_t nextIndex = ffStrbufNextIndexC(&buf, index, '\n');
        buf.chars[nextIndex] = '\0';
        const char* info = buf.chars + index;

        // {"Builtin display": uniqueId="local:4630947134992368259", 1440 x 3200, modeId 2, defaultModeId 1, supportedModes [{id=1, width=1440, height=3200, fps=60.000004, alternativeRefreshRates=[24.000002, 30.000002, 40.0, 120.00001, 120.00001, 120.00001, 120.00001, 120.00001]},
        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreateA(64);
        unsigned width = 0, height = 0, modeId = 0;
        double refreshRate = 0;
        // {"Builtin display": uniqueId="local:4630947134992368259", 1440 x 3200, modeId 2
        int res = sscanf(info, "{\"%63[^\"]\":%*s%u x %u, modeId%u", name.chars, &width, &height, &modeId);
        if (res >= 3) {
            if (res == 4) {
                ++info; // skip first '{'
                while ((info = strchr(info, '{'))) {
                    ++info;

                    unsigned id;
                    double fps;
                    // id=1, width=1440, height=3200, fps=60.000004,
                    if (sscanf(info, "id=%u, %*s%*s fps=%lf", &id, &fps) >= 2) {
                        if (id == modeId) {
                            refreshRate = fps;
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }

            ffStrbufRecalculateLength(&name);
            FFDisplayResult* display = ffdsAppendDisplay(ds,
                (uint32_t) width,
                (uint32_t) height,
                refreshRate,
                0,
                0,
                0,
                0,
                0,
                &name,
                FF_DISPLAY_TYPE_UNKNOWN,
                false,
                0,
                0,
                0,
                "dumpsys");
            if (display) {
                display->hdrStatus = checkHdrStatus(display);
            }
        }

        index = nextIndex + 1;
    }
}

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
        if (display) {
            display->hdrStatus = checkHdrStatus(display);
        }
        return !!display;
    }

    return false;
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
    // in `ro.vivo.product.version` (`PD2505D_A_9.16.42`).
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

    if (!detectWithGetprop(ds)) {
        detectWithDumpsys(ds);
    }

    detectDE(ds);
}
