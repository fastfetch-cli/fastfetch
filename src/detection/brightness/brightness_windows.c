#include "brightness.h"
#include "detection/displayserver/displayserver.h"
#include "common/debug.h"
#include "common/library.h"
#include "common/mallocHelper.h"
#include "common/windows/wmi.h"
#include "common/windows/unicode.h"

#include <inttypes.h>
#include <winternl.h>
#include <windows.h>

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
    FF_DEBUG("WMI: start detection");

    // https://github.com/tpn/winsdk-10/blob/master/Include/10.0.16299.0/km/wmicore.mof#L21200
    const GUID WmiMonitorBrightnessGuid = {
        0xd43412ac, 0x67f9, 0x4fbb, { 0xa0, 0x81, 0x17, 0x52, 0xa2, 0xc3, 0x3e, 0x84 }
    };

    FF_AUTO_CLOSE_WMI_BLOCK HANDLE hBlock = NULL;

    ULONG status = WmiOpenBlock(&WmiMonitorBrightnessGuid, WMIGUID_QUERY, &hBlock);
    if (status != 0) {
        FF_DEBUG("WMI: WmiOpenBlock failed: %s", ffDebugWin32Error(status));
        return "WmiOpenBlock() failed";
    }

    ULONG bufferSize = 0;
    status = WmiQueryAllDataW(hBlock, &bufferSize, NULL);
    if (status != ERROR_SUCCESS && status != ERROR_INSUFFICIENT_BUFFER) {
        FF_DEBUG("WMI: first WmiQueryAllDataW failed, bufferSize=%lu: %s", bufferSize, ffDebugWin32Error(status));
        return "WmiQueryAllDataW() failed";
    }

    FF_DEBUG("WMI: initial query bufferSize=%lu", bufferSize);

    if (bufferSize == 0) {
        FF_DEBUG("WMI: WmiQueryAllDataW returned empty buffer");
        return "WmiQueryAllDataW() returned no data";
    }

    FF_AUTO_FREE PWNODE_ALL_DATA pAllData = (PWNODE_ALL_DATA) malloc(bufferSize);

    status = WmiQueryAllDataW(hBlock, &bufferSize, pAllData);
    if (status != ERROR_SUCCESS) {
        FF_DEBUG("WMI: second WmiQueryAllDataW failed, bufferSize=%lu: %s", bufferSize, ffDebugWin32Error(status));
        return "WmiQueryAllDataW() failed";
    }

    if (bufferSize < sizeof(WNODE_ALL_DATA)) {
        FF_DEBUG("WMI: insufficient buffer for WNODE_ALL_DATA, bufferSize=%lu", bufferSize);
        return "WmiQueryAllDataW() returned insufficient data for WNODE_ALL_DATA";
    }

    FF_DEBUG("WMI: instanceCount=%lu, flags=0x%lX", pAllData->InstanceCount, pAllData->WnodeHeader.Flags);

    PULONG pNameOffsets = (PULONG) ((PUCHAR) pAllData + pAllData->OffsetInstanceNameOffsets);

    for (ULONG i = 0; i < pAllData->InstanceCount; i++) {
        ULONG dataOffset = 0;
        ULONG dataLength = 0;

        if (pAllData->WnodeHeader.Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE) {
            dataLength = pAllData->FixedInstanceSize;
            dataOffset = pAllData->DataBlockOffset + i * dataLength;
        } else {
            dataOffset = pAllData->OffsetInstanceDataAndLength[i].OffsetInstanceData;
            dataLength = pAllData->OffsetInstanceDataAndLength[i].LengthInstanceData;
        }

        if (dataLength == 0 || dataOffset >= bufferSize || dataLength > bufferSize - dataOffset) {
            FF_DEBUG("WMI: skip invalid instance %lu (dataOffset=%lu, dataLength=%lu, bufferSize=%lu)", i, dataOffset, dataLength, bufferSize);
            continue;
        }

        USHORT nameCharsCount = *(PUSHORT) ((PUCHAR) pAllData + pNameOffsets[i]) / sizeof(WCHAR);
        PCWSTR pNameChars = (PCWSTR) ((PUCHAR) pAllData + (pNameOffsets[i] + sizeof(USHORT)));

        PUCHAR pDataBlock = (PUCHAR) pAllData + dataOffset;
        UCHAR currentBrightness = pDataBlock[0];

        FFBrightnessResult* brightness = FF_LIST_ADD(FFBrightnessResult, *result);
        brightness->max = 100;
        brightness->min = 0;
        brightness->current = currentBrightness;
        brightness->builtin = true;
        ffStrbufInitNWS(&brightness->name, nameCharsCount, pNameChars);
        ffStrbufSubstrAfterFirstC(&brightness->name, '\\');
        ffStrbufSubstrBeforeFirstC(&brightness->name, '\\');

        FF_DEBUG("WMI: detected builtin display '%s', current=%u", brightness->name.chars, (unsigned) currentBrightness);
    }

    FF_DEBUG("WMI: finished detection, total results=%u", result->length);

    return NULL;
}

static const char* detectWithDdcci(const FFDisplayServerResult* displayServer, FFlist* result) {
    FF_DEBUG("DDC/CI: start detection, displayCount=%u", displayServer->displays.length);

    void* gdi32 = ffLibraryGetModule(L"gdi32.dll");
    if (!gdi32) {
        FF_DEBUG("DDC/CI: failed to load gdi32.dll");
        return "ffLibraryGetModule(gdi32.dll) failed";
    }
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(gdi32, GetPhysicalMonitors)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(gdi32, DDCCIGetVCPFeature)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(gdi32, DestroyPhysicalMonitorInternal)

    FF_LIST_FOR_EACH (FFDisplayResult, display, displayServer->displays) {
        if (display->type == FF_DISPLAY_TYPE_BUILTIN) {
            FF_DEBUG("DDC/CI: skip builtin display id=%" PRIu64, display->id);
            continue;
        }

        MONITORINFOEXW mi;
        mi.cbSize = sizeof(mi);
        if (!GetMonitorInfoW((HMONITOR) (uintptr_t) display->id, (LPMONITORINFO) &mi)) {
            FF_DEBUG("DDC/CI: GetMonitorInfoW failed for display id=%" PRIu64 ": %s", display->id, ffDebugWin32Error(GetLastError()));
            continue;
        }

        UNICODE_STRING deviceName = {
            .Length = (USHORT) (wcslen(mi.szDevice) * sizeof(wchar_t)),
            .MaximumLength = 0,
            .Buffer = mi.szDevice,
        };
        HANDLE physicalMonitor;
        DWORD monitorCount = 0;
        NTSTATUS monitorStatus = ffGetPhysicalMonitors(&deviceName, 1, &monitorCount, &physicalMonitor);
        if (NT_SUCCESS(monitorStatus) && monitorCount >= 1) {
            DWORD curr = 0, max = 0;
            if (NT_SUCCESS(ffDDCCIGetVCPFeature(physicalMonitor, 0x10 /* luminance */, NULL, &curr, &max))) {
                FFBrightnessResult* brightness = FF_LIST_ADD(FFBrightnessResult, *result);
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

                FF_DEBUG("DDC/CI: detected external display '%s', current=%u, max=%u", brightness->name.chars, (unsigned) curr, (unsigned) max);
            } else {
                FF_DEBUG("DDC/CI: DDCCIGetVCPFeature failed for monitor '%ls': %s", deviceName.Buffer, ffDebugWin32Error(GetLastError()));
            }

            ffDestroyPhysicalMonitorInternal(physicalMonitor);
        } else {
            FF_DEBUG("DDC/CI: GetPhysicalMonitors failed for '%ls', status=0x%08X, monitorCount=%lu, error=%s", deviceName.Buffer, (unsigned) monitorStatus, monitorCount, ffDebugWin32Error(GetLastError()));
        }
    }

    FF_DEBUG("DDC/CI: finished detection, total results=%u", result->length);
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

const char* ffDetectBrightness(FF_A_UNUSED FFBrightnessOptions* options, FFlist* result) {
    const FFDisplayServerResult* displayServer = ffConnectDisplayServer();
    FF_DEBUG("start, displayCount=%u", displayServer->displays.length);

    if (hasBuiltinDisplay(displayServer)) {
        FF_DEBUG("builtin display detected, trying WMI");
        detectWithWmi(result);
    }

    if (result->length < displayServer->displays.length) {
        FF_DEBUG("resultCount=%u < displayCount=%u, trying DDC/CI", result->length, displayServer->displays.length);
        detectWithDdcci(displayServer, result);
    }

    FF_DEBUG("finished, resultCount=%u", result->length);
    return NULL;
}
