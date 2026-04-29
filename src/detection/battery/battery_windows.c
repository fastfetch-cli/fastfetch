#define INITGUID

#include "battery.h"

#include "common/debug.h"
#include "common/mallocHelper.h"
#include "common/windows/unicode.h"
#include "common/windows/wmi.h"

#include <winerror.h>

typedef void(WINAPI* PINTERFACE_REFERENCE)(PVOID Context);
typedef void(WINAPI* PINTERFACE_DEREFERENCE)(PVOID Context);
typedef struct _DEVICE_OBJECT* PDEVICE_OBJECT;
typedef struct _IRP* PIRP;
#ifdef _WINDOWS_
    #undef _WINDOWS_
#endif

#include <batclass.h>

#pragma GCC diagnostic ignored "-Wmultichar"

typedef struct FFBatteryWmiEntry {
    ULONG tag;
    FFBatteryResult* result;
} FFBatteryWmiEntry;

static FFBatteryWmiEntry* getBatteryEntry(FFlist* entries, FFlist* results, ULONG tag) {
    FF_LIST_FOR_EACH (FFBatteryWmiEntry, entry, *entries) {
        if (entry->tag == tag) {
            return entry;
        }
    }

    FFBatteryWmiEntry* entry = FF_LIST_ADD(FFBatteryWmiEntry, *entries);
    entry->tag = tag;
    FFBatteryResult* battery = FF_LIST_ADD(FFBatteryResult, *results);
    entry->result = battery;
    ffStrbufInit(&battery->manufacturer);
    ffStrbufInit(&battery->manufactureDate);
    ffStrbufInit(&battery->modelName);
    ffStrbufInit(&battery->technology);
    ffStrbufInit(&battery->serial);
    battery->status = FF_BATTERY_STATUS_NONE;
    battery->capacity = -1;
    battery->temperature = FF_BATTERY_TEMP_UNSET;
    battery->cycleCount = 0;
    battery->timeRemaining = -1;
    return entry;
}

static const char* queryWmiAllData(const GUID* guid, const char* guidStr, PWNODE_ALL_DATA* pAllData, ULONG* pBufferSize) {
    FF_AUTO_CLOSE_WMI_BLOCK HANDLE hBlock = NULL;
    ULONG status = WmiOpenBlock(guid, WMIGUID_QUERY, &hBlock);
    if (status != ERROR_SUCCESS) {
        FF_DEBUG("WMI: WmiOpenBlock() failed for %s: %s", guidStr, ffDebugWin32Error(status));
        return "WmiOpenBlock() failed";
    }

    status = WmiQueryAllDataW(hBlock, pBufferSize, NULL);
    if (status != ERROR_SUCCESS && status != ERROR_INSUFFICIENT_BUFFER) {
        FF_DEBUG("WMI: first WmiQueryAllDataW() failed: %s", ffDebugWin32Error(status));
        return "WmiQueryAllDataW(NULL) failed";
    }

    if (*pBufferSize == 0) {
        return "WmiQueryAllDataW(NULL) returned no data";
    }

    if (*pBufferSize < sizeof(WNODE_ALL_DATA)) {
        FF_DEBUG("WMI: WmiQueryAllDataW() returned insufficient buffer size: %lu", *pBufferSize);
        return "WmiQueryAllDataW() returned insufficient data for WNODE_ALL_DATA";
    }

    *pAllData = (PWNODE_ALL_DATA) malloc(*pBufferSize);

    status = WmiQueryAllDataW(hBlock, pBufferSize, *pAllData);
    if (status != ERROR_SUCCESS) {
        FF_DEBUG("WMI: second WmiQueryAllDataW failed: %s", ffDebugWin32Error(status));
        free(*pAllData);
        *pAllData = NULL;
        return "WmiQueryAllDataW(*pAllData) failed";
    }

    return NULL;
}

static bool getInstanceData(const PWNODE_ALL_DATA allData, ULONG bufferSize, ULONG index, const uint8_t** instanceData, ULONG* instanceLength) {
    ULONG dataOffset = 0;
    ULONG dataLength = 0;

    if (allData->WnodeHeader.Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE) {
        dataLength = allData->FixedInstanceSize;
        dataOffset = allData->DataBlockOffset + index * dataLength;
    } else {
        dataOffset = allData->OffsetInstanceDataAndLength[index].OffsetInstanceData;
        dataLength = allData->OffsetInstanceDataAndLength[index].LengthInstanceData;
    }

    if (dataLength == 0 || dataOffset >= bufferSize || dataLength > bufferSize - dataOffset) {
        return false;
    }

    *instanceData = (const uint8_t*) allData + dataOffset;
    *instanceLength = dataLength;
    return true;
}

static void detectStaticData(FFlist* entries, FFlist* results) {
    FF_DEBUG("detectStaticData");
    FF_AUTO_FREE PWNODE_ALL_DATA allData = NULL;
    ULONG bufferSize = 0;
    const char* error = queryWmiAllData(&BATTERY_STATIC_DATA_WMI_GUID, "BATTERY_STATIC_DATA_WMI_GUID", &allData, &bufferSize);
    if (error) {
        return;
    }

    for (ULONG i = 0; i < allData->InstanceCount; ++i) {
        const uint8_t* instanceData = NULL;
        ULONG instanceLength = 0;
        if (!getInstanceData(allData, bufferSize, i, &instanceData, &instanceLength) || instanceLength < offsetof(BATTERY_WMI_STATIC_DATA, Strings)) {
            continue;
        }

        const BATTERY_WMI_STATIC_DATA* data = (const BATTERY_WMI_STATIC_DATA*) instanceData;
        FFBatteryWmiEntry* entry = getBatteryEntry(entries, results, data->Tag);

        FF_DEBUG("chemistry: %.4s", (const char*) &data->Chemistry);
        switch (data->Chemistry) {
            case 'cAbP':
                ffStrbufSetStatic(&entry->result->technology, "Lead Acid");
                break;
            case 'NOIL':
            case 'I-iL':
                ffStrbufSetStatic(&entry->result->technology, "Lithium Ion");
                break;
            case 'dCiN':
                ffStrbufSetStatic(&entry->result->technology, "Nickel Cadmium");
                break;
            case 'HMiN':
                ffStrbufSetStatic(&entry->result->technology, "Nickel Metal Hydride");
                break;
            case 'nZiN':
                ffStrbufSetStatic(&entry->result->technology, "Nickel Zinc");
                break;
            case '\0MAR':
                ffStrbufSetStatic(&entry->result->technology, "Rechargeable Alkaline-Manganese");
                break;
            default:
                ffStrbufSetStatic(&entry->result->technology, data->Technology ? "Rechargeable" : "Non Rechargeable");
                break;
        }

        const BATTERY_MANUFACTURE_DATE* manufactureDate = (const BATTERY_MANUFACTURE_DATE*) data->ManufactureDate;
        if (manufactureDate->Year > 0 && manufactureDate->Month >= 1 && manufactureDate->Month <= 12 && manufactureDate->Day >= 1 && manufactureDate->Day <= 31) {
            uint16_t year = manufactureDate->Year;
            ffStrbufSetF(&entry->result->manufactureDate, "%.4u-%.2u-%.2u", (unsigned) (year < 1000 ? (year + 1900) : year), (unsigned) manufactureDate->Month, (unsigned) manufactureDate->Day);
        }

        // Device Name, Manufacture Name, Serial Number, UniqueID
        const struct {
            uint16_t size; // in bytes, including the null terminator
            wchar_t value[];
        }* cursor = (const void*) data->Strings;

        FFstrbuf* strings[] = {
            &entry->result->modelName,
            &entry->result->manufacturer,
            &entry->result->serial,
        };

        for (size_t i = 0; i < ARRAY_SIZE(strings); ++i) {
            if (cursor->size > sizeof(wchar_t)) {
                ffStrbufSetNWS(strings[i], cursor->size / sizeof(wchar_t) - 1, cursor->value);
            }
            cursor = (const void*) ((const uint8_t*) cursor + sizeof(uint16_t) + cursor->size);
        }
    }
}

static void detectStatus(FFlist* entries, FFlist* results) {
    FF_DEBUG("detectStatus");
    FF_AUTO_FREE PWNODE_ALL_DATA allData = NULL;
    ULONG bufferSize = 0;
    const char* error = queryWmiAllData(&BATTERY_STATUS_WMI_GUID, "BATTERY_STATUS_WMI_GUID", &allData, &bufferSize);
    if (error) {
        return;
    }

    for (ULONG i = 0; i < allData->InstanceCount; ++i) {
        const uint8_t* instanceData = NULL;
        ULONG instanceLength = 0;
        if (!getInstanceData(allData, bufferSize, i, &instanceData, &instanceLength) || instanceLength < sizeof(BATTERY_WMI_STATUS)) {
            continue;
        }

        const BATTERY_WMI_STATUS* data = (const BATTERY_WMI_STATUS*) instanceData;
        FFBatteryWmiEntry* entry = getBatteryEntry(entries, results, data->Tag);
        if (data->RemainingCapacity != BATTERY_UNKNOWN_CAPACITY) {
            entry->result->capacity = data->RemainingCapacity;
        }

        entry->result->status = FF_BATTERY_STATUS_NONE;
        if (data->PowerOnline) {
            entry->result->status |= FF_BATTERY_STATUS_AC_CONNECTED;
        }
        if (data->Charging) {
            entry->result->status |= FF_BATTERY_STATUS_CHARGING;
        }
        if (data->Discharging) {
            entry->result->status |= FF_BATTERY_STATUS_DISCHARGING;
        }
        if (data->Critical) {
            entry->result->status |= FF_BATTERY_STATUS_CRITICAL;
        }
    }
}

static void detectRuntime(FFlist* entries, FFlist* results) {
    FF_DEBUG("detectRuntime");
    FF_AUTO_FREE PWNODE_ALL_DATA allData = NULL;
    ULONG bufferSize = 0;
    const char* error = queryWmiAllData(&BATTERY_RUNTIME_WMI_GUID, "BATTERY_RUNTIME_WMI_GUID", &allData, &bufferSize);
    if (error) {
        return;
    }

    for (ULONG i = 0; i < allData->InstanceCount; ++i) {
        const uint8_t* instanceData = NULL;
        ULONG instanceLength = 0;
        if (!getInstanceData(allData, bufferSize, i, &instanceData, &instanceLength) || instanceLength < sizeof(BATTERY_WMI_RUNTIME)) {
            continue;
        }

        const BATTERY_WMI_RUNTIME* data = (const BATTERY_WMI_RUNTIME*) instanceData;
        FFBatteryWmiEntry* entry = getBatteryEntry(entries, results, data->Tag);
        if (data->EstimatedRuntime != BATTERY_UNKNOWN_TIME) {
            entry->result->timeRemaining = (int32_t) data->EstimatedRuntime;
        }
    }
}

static void detectFullChargedCapacity(FFlist* entries, FFlist* results) {
    FF_DEBUG("detectFullChargedCapacity");
    FF_AUTO_FREE PWNODE_ALL_DATA allData = NULL;
    ULONG bufferSize = 0;
    const char* error = queryWmiAllData(&BATTERY_FULL_CHARGED_CAPACITY_WMI_GUID, "BATTERY_FULL_CHARGED_CAPACITY_WMI_GUID", &allData, &bufferSize);
    if (error) {
        return;
    }

    for (ULONG i = 0; i < allData->InstanceCount; ++i) {
        const uint8_t* instanceData = NULL;
        ULONG instanceLength = 0;
        if (!getInstanceData(allData, bufferSize, i, &instanceData, &instanceLength) || instanceLength < sizeof(BATTERY_WMI_FULL_CHARGED_CAPACITY)) {
            continue;
        }

        const BATTERY_WMI_FULL_CHARGED_CAPACITY* data = (const BATTERY_WMI_FULL_CHARGED_CAPACITY*) instanceData;
        FFBatteryWmiEntry* entry = getBatteryEntry(entries, results, data->Tag);

        if (data->FullChargedCapacity != BATTERY_UNKNOWN_CAPACITY && entry->result->capacity >= 0) {
            entry->result->capacity *= 100;
            entry->result->capacity /= data->FullChargedCapacity;
        }
    }
}

static void detectCycleCount(FFlist* entries, FFlist* results) {
    FF_DEBUG("detectCycleCount");
    FF_AUTO_FREE PWNODE_ALL_DATA allData = NULL;
    ULONG bufferSize = 0;
    const char* error = queryWmiAllData(&BATTERY_CYCLE_COUNT_WMI_GUID, "BATTERY_CYCLE_COUNT_WMI_GUID", &allData, &bufferSize);
    if (error) {
        return;
    }

    for (ULONG i = 0; i < allData->InstanceCount; ++i) {
        const uint8_t* instanceData = NULL;
        ULONG instanceLength = 0;
        if (!getInstanceData(allData, bufferSize, i, &instanceData, &instanceLength) || instanceLength < sizeof(BATTERY_WMI_CYCLE_COUNT)) {
            continue;
        }

        const BATTERY_WMI_CYCLE_COUNT* data = (const BATTERY_WMI_CYCLE_COUNT*) instanceData;
        getBatteryEntry(entries, results, data->Tag)->result->cycleCount = data->CycleCount;
    }
}

static void detectTemperature(FFlist* entries, FFlist* results) {
    FF_DEBUG("detectTemperature");
    FF_AUTO_FREE PWNODE_ALL_DATA allData = NULL;
    ULONG bufferSize = 0;
    const char* error = queryWmiAllData(&BATTERY_TEMPERATURE_WMI_GUID, "BATTERY_TEMPERATURE_WMI_GUID", &allData, &bufferSize);
    if (error) {
        return;
    }

    for (ULONG i = 0; i < allData->InstanceCount; ++i) {
        const uint8_t* instanceData = NULL;
        ULONG instanceLength = 0;
        if (!getInstanceData(allData, bufferSize, i, &instanceData, &instanceLength) || instanceLength < sizeof(BATTERY_WMI_TEMPERATURE)) {
            continue;
        }

        const BATTERY_WMI_TEMPERATURE* data = (const BATTERY_WMI_TEMPERATURE*) instanceData;
        getBatteryEntry(entries, results, data->Tag)->result->temperature = data->Temperature / 10.0 - 273.15;
    }
}

static const char* detectWithNtApi(FFBatteryResult* battery) {
    // Reports summary battery information, not per battery
    FF_DEBUG("NtApi: start detection");
    SYSTEM_BATTERY_STATE info;
    NTSTATUS status = NtPowerInformation(SystemBatteryState, NULL, 0, &info, sizeof(info));
    if (!NT_SUCCESS(status)) {
        FF_DEBUG("NtApi: NtPowerInformation(SystemBatteryState) failed: %s", ffDebugNtStatus(status));
        return "NtPowerInformation(SystemBatteryState) failed";
    }
    if (!info.BatteryPresent) {
        FF_DEBUG("NtApi reports no battery present");
        return "No battery present";
    }

    if (info.MaxCapacity != BATTERY_UNKNOWN_CAPACITY && info.RemainingCapacity != BATTERY_UNKNOWN_CAPACITY) {
        battery->capacity = info.RemainingCapacity * 100.0 / info.MaxCapacity;
    }
    battery->status = FF_BATTERY_STATUS_NONE;
    if (info.AcOnLine) {
        battery->status |= FF_BATTERY_STATUS_AC_CONNECTED;
    }
    if (info.Charging) {
        battery->status |= FF_BATTERY_STATUS_CHARGING;
    }
    if (info.Discharging) {
        battery->status |= FF_BATTERY_STATUS_DISCHARGING;
    }
    if (info.DefaultAlert1 > 0 && info.RemainingCapacity <= info.DefaultAlert1) {
        battery->status |= FF_BATTERY_STATUS_CRITICAL;
    }
    battery->timeRemaining = info.EstimatedTime == BATTERY_UNKNOWN_TIME ? -1 : (int32_t) info.EstimatedTime;
    return NULL;
}

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results) {
    FF_DEBUG("WMI: start detection");

    FF_LIST_AUTO_DESTROY entries = ffListCreate();
    detectStaticData(&entries, results);
    if (results->length == 0) {
        return NULL;
    } else if (results->length == 1) {
        // Fast path for single battery
        detectWithNtApi(FF_LIST_FIRST(FFBatteryWmiEntry, entries)->result);
    } else {
        detectStatus(&entries, results);
        detectFullChargedCapacity(&entries, results);
        detectRuntime(&entries, results);
    }
    detectCycleCount(&entries, results);
    if (options->temp) {
        detectTemperature(&entries, results);
    }

    FF_LIST_FOR_EACH (FFBatteryWmiEntry, entry, entries) {
        FF_DEBUG(
            "WMI: detected battery tag=%lu, name='%s', charge=%.2f%%, status=0x%x, runtime=%d seconds",
            entry->tag,
            entry->result->modelName.length ? entry->result->modelName.chars : "<unknown>",
            entry->result->capacity,
            entry->result->status,
            entry->result->timeRemaining);
    }

    FF_DEBUG("WMI: finished detection, total results=%u", results->length);
    return NULL;
}
