#include "displayserver.h"
#include "common/edidHelper.h"
#include "common/windows/registry.h"
#include "common/windows/unicode.h"

#include <windows.h>
#include <shellscalingapi.h>

static inline void freeArgBuffer(FFArgBuffer* buffer) {
    if (buffer->data) {
        free(buffer->data);
    }
    buffer->data = NULL;
    buffer->length = 0;
}
#define FF_AUTO_FREE_ARG_BUFFER __attribute__((__cleanup__(freeArgBuffer)))

// http://undoc.airesoft.co.uk/user32.dll/IsThreadDesktopComposited.php
BOOL WINAPI IsThreadDesktopComposited();
BOOL WINAPI GetDpiForMonitorInternal(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);

static void detectDisplays(FFDisplayServerResult* ds) {
    DISPLAYCONFIG_PATH_INFO paths[128];
    uint32_t pathCount = ARRAY_SIZE(paths);
    DISPLAYCONFIG_MODE_INFO modes[256];
    uint32_t modeCount = ARRAY_SIZE(modes);

    if (QueryDisplayConfig(
            QDC_ONLY_ACTIVE_PATHS,
            &pathCount,
            paths,
            &modeCount,
            modes,
            NULL) == ERROR_SUCCESS) {
        for (uint32_t i = 0; i < pathCount; ++i) {
            const DISPLAYCONFIG_PATH_INFO* path = &paths[i];
            const DISPLAYCONFIG_SOURCE_MODE* sourceMode = &modes[path->sourceInfo.modeInfoIdx].sourceMode;

            FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
            uint32_t physicalWidth = 0, physicalHeight = 0;

            DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {
                .header = {
                    .type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME,
                    .size = sizeof(targetName),
                    .adapterId = path->targetInfo.adapterId,
                    .id = path->targetInfo.id,
                },
            };
            FF_AUTO_FREE_ARG_BUFFER FFArgBuffer edid = {};
            if (DisplayConfigGetDeviceInfo(&targetName.header) == ERROR_SUCCESS) {
                wchar_t regPath[256] = L"SYSTEM\\CurrentControlSet\\Enum";
                wchar_t* pRegPath = regPath + strlen("SYSTEM\\CurrentControlSet\\Enum");
                wchar_t* pDevPath = targetName.monitorDevicePath + strlen("\\\\?");
                while (*pDevPath && *pDevPath != L'{') {
                    if (*pDevPath == L'#') {
                        *pRegPath = L'\\';
                    } else {
                        *pRegPath = *pDevPath;
                    }
                    ++pRegPath;
                    ++pDevPath;
                    assert(pRegPath < regPath + ARRAY_SIZE(regPath) + strlen("Device Parameters"));
                }
                wcscpy(pRegPath, L"Device Parameters");

                FF_AUTO_CLOSE_FD HANDLE hKey = NULL;
                if (ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regPath, &hKey, NULL) &&
                    ffRegReadData(hKey, L"EDID", &edid, NULL) &&
                    ffEdidIsValid(edid.data, edid.length)) {
                    ffEdidGetName(edid.data, &name);
                    ffEdidGetPhysicalSize(edid.data, &physicalWidth, &physicalHeight);
                } else {
                    edid.length = 0;
                    if (targetName.flags.friendlyNameFromEdid) {
                        ffStrbufSetWS(&name, targetName.monitorFriendlyDeviceName);
                    } else {
                        ffStrbufSetWS(&name, targetName.monitorDevicePath);
                        ffStrbufSubstrAfterFirstC(&name, '#');
                        ffStrbufSubstrBeforeFirstC(&name, '#');
                    }
                }
            }

            uint32_t width = sourceMode->width;
            uint32_t height = sourceMode->height;
            uint32_t rotation;
            switch (path->targetInfo.rotation) {
                case DISPLAYCONFIG_ROTATION_ROTATE90:
                    rotation = 90;
                    break;
                case DISPLAYCONFIG_ROTATION_ROTATE180:
                    rotation = 180;
                    break;
                case DISPLAYCONFIG_ROTATION_ROTATE270:
                    rotation = 270;
                    break;
                default:
                    rotation = 0;
                    break;
            }

            DISPLAYCONFIG_TARGET_PREFERRED_MODE preferredMode = {
                .header = {
                    .type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_PREFERRED_MODE,
                    .size = sizeof(preferredMode),
                    .adapterId = path->targetInfo.adapterId,
                    .id = path->targetInfo.id,
                }
            };
            double preferredRefreshRate = 0;
            if (DisplayConfigGetDeviceInfo(&preferredMode.header) == ERROR_SUCCESS) {
                DISPLAYCONFIG_RATIONAL freq = preferredMode.targetMode.targetVideoSignalInfo.vSyncFreq;
                preferredRefreshRate = freq.Numerator / (double) freq.Denominator;
            }

            uint32_t systemDpi = 0;
            HMONITOR hMonitor = MonitorFromPoint(*(POINT*) &sourceMode->position, MONITOR_DEFAULTTONULL);
            if (hMonitor) {
                UINT ignored;
                GetDpiForMonitorInternal(hMonitor, MDT_EFFECTIVE_DPI, &systemDpi, &ignored);
            }

            if (systemDpi == 0) {
                HDC hdc = GetDC(NULL);
                systemDpi = (uint32_t) GetDeviceCaps(hdc, LOGPIXELSX);
                if (systemDpi == 0) {
                    systemDpi = 96;
                }
                ReleaseDC(NULL, hdc);
            }

            if (path->targetInfo.rotation == DISPLAYCONFIG_ROTATION_ROTATE90 ||
                path->targetInfo.rotation == DISPLAYCONFIG_ROTATION_ROTATE270) {
                uint32_t temp = width;
                width = height;
                height = temp;
            }

            FFDisplayResult* display = ffdsAppendDisplay(ds,
                width,
                height,
                path->targetInfo.refreshRate.Numerator / (double) path->targetInfo.refreshRate.Denominator,
                systemDpi,
                preferredMode.width,
                preferredMode.height,
                preferredRefreshRate,
                rotation,
                &name,
                path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER ? FF_DISPLAY_TYPE_UNKNOWN : path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL || path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED || path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED ? FF_DISPLAY_TYPE_BUILTIN
                                                                                                                                                                                                                                                                                                                                                                                       : FF_DISPLAY_TYPE_EXTERNAL,
                sourceMode->position.x == 0 && sourceMode->position.y == 0,
                (uintptr_t) hMonitor,
                physicalWidth,
                physicalHeight,
                "GDI");

            if (display) {
                DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO_2 advColorInfo2 = {
                    .header = {
                        .type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO_2,
                        .size = sizeof(advColorInfo2),
                        .adapterId = path->targetInfo.adapterId,
                        .id = path->targetInfo.id,
                    }
                };
                if (DisplayConfigGetDeviceInfo(&advColorInfo2.header) == ERROR_SUCCESS) {
                    if (advColorInfo2.highDynamicRangeUserEnabled) {
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_ENABLED;
                    } else if (advColorInfo2.highDynamicRangeSupported) {
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_SUPPORTED;
                    } else {
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
                    }
                    display->bitDepth = (uint8_t) advColorInfo2.bitsPerColorChannel;
                } else {
                    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO advColorInfo = {
                        .header = {
                            .type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO,
                            .size = sizeof(advColorInfo),
                            .adapterId = path->targetInfo.adapterId,
                            .id = path->targetInfo.id,
                        }
                    };
                    if (DisplayConfigGetDeviceInfo(&advColorInfo.header) == ERROR_SUCCESS) {
                        if (advColorInfo.advancedColorEnabled) {
                            display->hdrStatus = FF_DISPLAY_HDR_STATUS_ENABLED;
                        } else if (advColorInfo.advancedColorSupported) {
                            display->hdrStatus = FF_DISPLAY_HDR_STATUS_SUPPORTED;
                        } else {
                            display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
                        }
                        display->bitDepth = (uint8_t) advColorInfo.bitsPerColorChannel;
                    } else {
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNKNOWN;
                    }
                }
                if (edid.length > 0) {
                    ffEdidGetSerialAndManufactureDate(edid.data, &display->serial, &display->manufactureYear, &display->manufactureWeek);
                }
                display->drrStatus = path->flags & DISPLAYCONFIG_PATH_BOOST_REFRESH_RATE ? FF_DISPLAY_DRR_STATUS_ENABLED : FF_DISPLAY_DRR_STATUS_DISABLED;
            }
        }
    }
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds) {
    if (IsThreadDesktopComposited()) {
        ffStrbufSetStatic(&ds->wmProcessName, "dwm.exe");
        ffStrbufSetStatic(&ds->wmPrettyName, "Desktop Window Manager");
    } else {
        // `explorer.exe` only provides a subset of WM functions, as well as the taskbar and desktop icons.
        // While a window itself is drawn by kernel (GDI). Killing `explorer.exe` won't affect how windows are displayed generally.
        ffStrbufSetStatic(&ds->wmProcessName, "explorer.exe");
        ffStrbufSetStatic(&ds->wmPrettyName, "Internal");
    }

    detectDisplays(ds);
}
