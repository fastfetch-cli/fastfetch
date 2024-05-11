#include "battery.h"

#include "common/io/io.h"
#include "util/windows/unicode.h"
#include "util/mallocHelper.h"
#include "util/smbiosHelper.h"

#undef WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <setupapi.h>
#include <batclass.h>
#include <devguid.h>
#include <winternl.h>

NTSYSCALLAPI
NTSTATUS
NTAPI
NtPowerInformation(
    IN POWER_INFORMATION_LEVEL InformationLevel,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength);

static inline void wrapSetupDiDestroyDeviceInfoList(HDEVINFO* hdev)
{
    if(*hdev)
        SetupDiDestroyDeviceInfoList(*hdev);
}

static const char* detectWithSetupApi(FFBatteryOptions* options, FFlist* results)
{
    //https://learn.microsoft.com/en-us/windows/win32/power/enumerating-battery-devices
    HDEVINFO hdev __attribute__((__cleanup__(wrapSetupDiDestroyDeviceInfoList))) =
        SetupDiGetClassDevsW(&GUID_DEVCLASS_BATTERY, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if(hdev == INVALID_HANDLE_VALUE)
        return "SetupDiGetClassDevsW(&GUID_DEVCLASS_BATTERY) failed";

    SP_DEVICE_INTERFACE_DATA did = { .cbSize = sizeof(did) };
    for(DWORD idev = 0; SetupDiEnumDeviceInterfaces(hdev, NULL, &GUID_DEVCLASS_BATTERY, idev, &did); idev++)
    {
        DWORD cbRequired = 0;
        SetupDiGetDeviceInterfaceDetailW(hdev, &did, NULL, 0, &cbRequired, NULL); //Fail with not enough buffer
        SP_DEVICE_INTERFACE_DETAIL_DATA_W* FF_AUTO_FREE pdidd = (SP_DEVICE_INTERFACE_DETAIL_DATA_W*)malloc(cbRequired);
        if(!pdidd)
            break; //Out of memory

        pdidd->cbSize = sizeof(*pdidd);
        if(!SetupDiGetDeviceInterfaceDetailW(hdev, &did, pdidd, cbRequired, &cbRequired, NULL))
            continue;

        HANDLE FF_AUTO_CLOSE_FD hBattery =
            CreateFileW(pdidd->DevicePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if(hBattery == INVALID_HANDLE_VALUE)
            continue;

        BATTERY_QUERY_INFORMATION bqi = { .InformationLevel = BatteryInformation };

        DWORD dwWait = 0;
        DWORD dwOut;

        if(!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_TAG, &dwWait, sizeof(dwWait), &bqi.BatteryTag, sizeof(bqi.BatteryTag), &dwOut, NULL) && bqi.BatteryTag)
            continue;

        BATTERY_INFORMATION bi = {0};
        if(!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi), &bi, sizeof(bi), &dwOut, NULL))
            continue;

        if(!(bi.Capabilities & BATTERY_SYSTEM_BATTERY))
            continue;

        FFBatteryResult* battery = (FFBatteryResult*)ffListAdd(results);

        if(memcmp(bi.Chemistry, "PbAc", 4) == 0)
            ffStrbufInitStatic(&battery->technology, "Lead Acid");
        else if(memcmp(bi.Chemistry, "LION", 4) == 0 || memcmp(bi.Chemistry, "Li-I", 4) == 0)
            ffStrbufInitStatic(&battery->technology, "Lithium Ion");
        else if(memcmp(bi.Chemistry, "NiCd", 4) == 0)
            ffStrbufInitStatic(&battery->technology, "Nickel Cadmium");
        else if(memcmp(bi.Chemistry, "NiMH", 4) == 0)
            ffStrbufInitStatic(&battery->technology, "Nickel Metal Hydride");
        else if(memcmp(bi.Chemistry, "NiZn", 4) == 0)
            ffStrbufInitStatic(&battery->technology, "Nickel Zinc");
        else if(memcmp(bi.Chemistry, "RAM\0", 4) == 0)
            ffStrbufInitStatic(&battery->technology, "Rechargeable Alkaline-Manganese");
        else
            ffStrbufInitStatic(&battery->technology, "Unknown");

        {
            ffStrbufInit(&battery->modelName);
            bqi.InformationLevel = BatteryDeviceName;
            wchar_t name[64];
            if(DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi), name, sizeof(name), &dwOut, NULL))
                ffStrbufSetWS(&battery->modelName, name);
        }

        {
            ffStrbufInit(&battery->manufacturer);
            bqi.InformationLevel = BatteryManufactureName;
            wchar_t name[64];
            if(DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi), name, sizeof(name), &dwOut, NULL))
                ffStrbufSetWS(&battery->manufacturer, name);
        }

        {
            ffStrbufInit(&battery->manufactureDate);
            bqi.InformationLevel = BatteryManufactureDate;
            BATTERY_MANUFACTURE_DATE date;
            if(DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi), &date, sizeof(date), &dwOut, NULL))
                ffStrbufSetF(&battery->manufactureDate, "%.4d-%.2d-%.2d", date.Year + 1900, date.Month, date.Day);
        }

        {
            ffStrbufInit(&battery->serial);
            bqi.InformationLevel = BatterySerialNumber;
            wchar_t name[64];
            if(DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi), name, sizeof(name), &dwOut, NULL))
                ffStrbufSetWS(&battery->serial, name);
        }

        battery->cycleCount = bi.CycleCount;

        battery->temperature = 0.0/0.0;
        if(options->temp)
        {
            bqi.InformationLevel = BatteryTemperature;
            ULONG temp;
            if(DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi), &temp, sizeof(temp), &dwOut, NULL))
                battery->temperature = temp / 10.0 - 273.15;
        }

        {
            BATTERY_STATUS bs;
            BATTERY_WAIT_STATUS bws = { .BatteryTag = bqi.BatteryTag };
            if(DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_STATUS, &bws, sizeof(bws), &bs, sizeof(bs), &dwOut, NULL) && bs.Capacity != BATTERY_UNKNOWN_CAPACITY)
                battery->capacity = bs.Capacity * 100.0 / bi.FullChargedCapacity;
            else
                battery->capacity = 0;

            ffStrbufInit(&battery->status);
            if(bs.PowerState & BATTERY_POWER_ON_LINE)
                ffStrbufAppendS(&battery->status, "AC Connected, ");
            if(bs.PowerState & BATTERY_DISCHARGING)
                ffStrbufAppendS(&battery->status, "Discharging, ");
            if(bs.PowerState & BATTERY_CHARGING)
                ffStrbufAppendS(&battery->status, "Charging, ");
            if(bs.PowerState & BATTERY_CRITICAL)
                ffStrbufAppendS(&battery->status, "Critical, ");
            ffStrbufTrimRight(&battery->status, ' ');
            ffStrbufTrimRight(&battery->status, ',');
        }
    }
    return NULL;
}

typedef struct FFSmbiosPortableBattery
{
    FFSmbiosHeader Header;

    // 2.1+
    uint8_t Location; // string
    uint8_t Manufacturer; // string
    uint8_t ManufactureDate; // string
    uint8_t SerialNumber; // string
    uint8_t DeviceName; // string
    uint8_t DeviceChemistry; // enum
    uint16_t DesignCapacity; // varies
    uint16_t DesignVoltage; // varies
    uint8_t SbdsVersionNumber; // string
    uint8_t MaximumErrorInBatteryData; // varies

    // 2.2+
    uint16_t SbdsSerialNumber; // varies
    uint16_t SbdsManufactureDate; // varies
    uint8_t SbdsDeviceChemistry; // string
    uint8_t DesignCapacityMultiplier; // varies
    uint16_t OEMSpecific; // varies
} __attribute__((__packed__)) FFSmbiosPortableBattery;

static_assert(offsetof(FFSmbiosPortableBattery, OEMSpecific) == 0x16,
    "FFSmbiosPortableBattery: Wrong struct alignment");

const char* detectBySmbios(FFBatteryResult* battery)
{
    const FFSmbiosHeaderTable* smbiosTable = ffGetSmbiosHeaderTable();
    if (!smbiosTable)
        return "Failed to get SMBIOS data";

    const FFSmbiosPortableBattery* data = (const FFSmbiosPortableBattery*) (*smbiosTable)[FF_SMBIOS_TYPE_PORTABLE_BATTERY];
    if (!data)
        return "Portable battery section is not found in SMBIOS data";

    const char* strings = (const char*) data + data->Header.Length;

    ffStrbufSetStatic(&battery->modelName, ffSmbiosLocateString(strings, data->DeviceName));
    ffCleanUpSmbiosValue(&battery->modelName);
    ffStrbufSetStatic(&battery->manufacturer, ffSmbiosLocateString(strings, data->Manufacturer));
    ffCleanUpSmbiosValue(&battery->manufacturer);

    if (data->ManufactureDate)
    {
        ffStrbufSetStatic(&battery->manufactureDate, ffSmbiosLocateString(strings, data->ManufactureDate));
        ffCleanUpSmbiosValue(&battery->manufactureDate);
    }
    else if (data->Header.Length > offsetof(FFSmbiosPortableBattery, SbdsManufactureDate))
    {
        int day = data->SbdsManufactureDate & 0b11111;
        int month = (data->SbdsManufactureDate >> 5) & 0b1111;
        int year = (data->SbdsManufactureDate >> 9) + 1800;
        ffStrbufSetF(&battery->manufactureDate, "%.4d-%.2d-%.2d", year, month, day);
    }

    switch (data->DeviceChemistry)
    {
    case 0x01: ffStrbufSetStatic(&battery->technology, "Other"); break;
    case 0x02: ffStrbufSetStatic(&battery->technology, "Unknown"); break;
    case 0x03: ffStrbufSetStatic(&battery->technology, "Lead Acid"); break;
    case 0x04: ffStrbufSetStatic(&battery->technology, "Nickel Cadmium"); break;
    case 0x05: ffStrbufSetStatic(&battery->technology, "Nickel metal hydride"); break;
    case 0x06: ffStrbufSetStatic(&battery->technology, "Lithium-ion"); break;
    case 0x07: ffStrbufSetStatic(&battery->technology, "Zinc air"); break;
    case 0x08: ffStrbufSetStatic(&battery->technology, "Lithium Polymer"); break;
    }

    if (data->SerialNumber)
    {
        ffStrbufSetStatic(&battery->serial, ffSmbiosLocateString(strings, data->SerialNumber));
        ffCleanUpSmbiosValue(&battery->serial);
    }
    else if (data->Header.Length > offsetof(FFSmbiosPortableBattery, SbdsSerialNumber))
    {
        ffStrbufSetF(&battery->serial, "%4X", data->SbdsSerialNumber);
    }

    return NULL;
}

static const char* detectWithNtApi(FF_MAYBE_UNUSED FFBatteryOptions* options, FFlist* results)
{
    SYSTEM_BATTERY_STATE info;
    if (NT_SUCCESS(NtPowerInformation(SystemBatteryState, NULL, 0, &info, sizeof(info))) && info.BatteryPresent)
    {
        FFBatteryResult* battery = (FFBatteryResult*)ffListAdd(results);
        ffStrbufInit(&battery->modelName);
        ffStrbufInit(&battery->manufacturer);
        ffStrbufInit(&battery->manufactureDate);
        ffStrbufInit(&battery->technology);
        ffStrbufInit(&battery->status);
        ffStrbufInit(&battery->serial);
        battery->temperature = FF_BATTERY_TEMP_UNSET;
        battery->cycleCount = 0;

        battery->capacity = info.RemainingCapacity * 100.0 / info.MaxCapacity;
        if(info.AcOnLine)
        {
            ffStrbufAppendS(&battery->status, "AC Connected");
            if(info.Charging)
                ffStrbufAppendS(&battery->status, ", Charging");
        }
        else if(info.Discharging)
            ffStrbufAppendS(&battery->status, "Discharging");

        detectBySmbios(battery);

        return NULL;
    }
    return "NtPowerInformation(SystemBatteryState) failed";
}

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results)
{
    return options->useSetupApi
        ? detectWithSetupApi(options, results)
        : detectWithNtApi(options, results);
}
