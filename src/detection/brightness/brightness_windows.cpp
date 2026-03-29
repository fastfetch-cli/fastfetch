extern "C" {
#include "brightness.h"
#include "detection/displayserver/displayserver.h"
#include "common/library.h"
}
#include "common/windows/wmi.hpp"
#include "common/windows/unicode.hpp"

#include <winternl.h>

NTSYSAPI NTSTATUS WINAPI GetPhysicalMonitors(
    _In_ UNICODE_STRING* pstrDeviceName,
    _In_ DWORD dwPhysicalMonitorArraySize,
    _Out_ DWORD* pdwNumPhysicalMonitorHandlesInArray,
    _Out_ HANDLE* phPhysicalMonitorArray);

typedef enum _MC_VCP_CODE_TYPE {
    MC_MOMENTARY,
    MC_SET_PARAMETER
} MC_VCP_CODE_TYPE,
    *LPMC_VCP_CODE_TYPE;

NTSYSAPI NTSTATUS WINAPI DDCCIGetVCPFeature(
    _In_ HANDLE hMonitor,
    _In_ DWORD dwVCPCode,
    _Out_opt_ LPMC_VCP_CODE_TYPE pvct,
    _Out_ DWORD* pdwCurrentValue,
    _Out_opt_ DWORD* pdwMaximumValue);

NTSYSAPI NTSTATUS WINAPI DestroyPhysicalMonitorInternal(
    _In_ HANDLE hMonitor);

NTSTATUS WINAPI GetPhysicalMonitorDescription(
    _In_ HANDLE hMonitor,
    _In_ DWORD dwPhysicalMonitorDescriptionSizeInChars,
    _Out_ LPWSTR szPhysicalMonitorDescription);

static const char* detectWithWmi(FFlist* result) {
    FFWmiQuery query(L"SELECT CurrentBrightness, InstanceName FROM WmiMonitorBrightness WHERE Active = true", nullptr, FFWmiNamespace::WMI);
    if (!query) {
        return "Query WMI service failed";
    }

    while (FFWmiRecord record = query.next()) {
        if (FFWmiVariant vtValue = record.get(L"CurrentBrightness")) {
            FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
            brightness->max = 100;
            brightness->min = 0;
            brightness->current = vtValue.get<uint8_t>();
            brightness->builtin = true;

            ffStrbufInit(&brightness->name);
            if (FFWmiVariant vtName = record.get(L"InstanceName")) {
                ffStrbufSetWSV(&brightness->name, vtName.get<std::wstring_view>());
                ffStrbufSubstrAfterFirstC(&brightness->name, '\\');
                ffStrbufSubstrBeforeFirstC(&brightness->name, '\\');
            }
        }
    }
    return NULL;
}

static const char* detectWithDdcci(const FFDisplayServerResult* displayServer, FFlist* result) {
    void* gdi32 = ffLibraryGetModule(L"gdi32.dll");
    if (!gdi32) {
        return "ffLibraryGetModule(gdi32.dll) failed";
    }
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(gdi32, GetPhysicalMonitors)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(gdi32, DDCCIGetVCPFeature)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(gdi32, DestroyPhysicalMonitorInternal)

    FF_LIST_FOR_EACH (FFDisplayResult, display, displayServer->displays) {
        if (display->type == FF_DISPLAY_TYPE_BUILTIN) {
            continue;
        }

        MONITORINFOEXW mi;
        mi.cbSize = sizeof(mi);
        if (!GetMonitorInfoW((HMONITOR) (uintptr_t) display->id, (LPMONITORINFO) &mi)) {
            continue;
        }

        UNICODE_STRING deviceName = {
            .Length = (USHORT) (wcslen(mi.szDevice) * sizeof(wchar_t)),
            .MaximumLength = 0,
            .Buffer = mi.szDevice,
        };
        HANDLE physicalMonitor;
        DWORD monitorCount = 0;
        if (NT_SUCCESS(ffGetPhysicalMonitors(&deviceName, 1, &monitorCount, &physicalMonitor)) && monitorCount >= 1) {
            DWORD curr = 0, max = 0;
            if (NT_SUCCESS(ffDDCCIGetVCPFeature(physicalMonitor, 0x10 /* luminance */, NULL, &curr, &max))) {
                FFBrightnessResult* brightness = (FFBrightnessResult*) ffListAdd(result);
                if (display->name.length > 0) {
                    ffStrbufInitCopy(&brightness->name, &display->name);
                } else {
                    FF_LIBRARY_LOAD_SYMBOL_LAZY(gdi32, GetPhysicalMonitorDescription)
                    if (ffGetPhysicalMonitorDescription) {
                        wchar_t description[128 /*MUST be PHYSICAL_MONITOR_DESCRIPTION_SIZE*/];
                        if (NT_SUCCESS(ffGetPhysicalMonitorDescription(physicalMonitor, ARRAY_SIZE(description), description))) {
                            ffStrbufInitWS(&brightness->name, description);
                        }
                    }
                    if (brightness->name.length == 0) {
                        ffStrbufSetNWS(&brightness->name, deviceName.Length / 2, deviceName.Buffer);
                    }
                }
                brightness->max = max;
                brightness->min = 0;
                brightness->current = curr;
                brightness->builtin = false;
            }

            ffDestroyPhysicalMonitorInternal(physicalMonitor);
        }
    }
    return NULL;
}

static bool hasBuiltinDisplay(const FFDisplayServerResult* displayServer) {
    FF_LIST_FOR_EACH (FFDisplayResult, display, displayServer->displays) {
        if (display->type == FF_DISPLAY_TYPE_BUILTIN || display->type == FF_DISPLAY_TYPE_UNKNOWN) {
            return true;
        }
    }
    return false;
}

extern "C" const char* ffDetectBrightness(FF_MAYBE_UNUSED FFBrightnessOptions* options, FFlist* result) {
    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();

    if (hasBuiltinDisplay(displayServer)) {
        detectWithWmi(result);
    }

    if (result->length < displayServer->displays.length) {
        detectWithDdcci(displayServer, result);
    }
    return NULL;
}
