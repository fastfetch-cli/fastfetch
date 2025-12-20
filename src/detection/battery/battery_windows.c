#include "battery.h"

#include "common/io/io.h"
#include "util/windows/nt.h"
#include "util/windows/unicode.h"
#include "util/mallocHelper.h"
#include "util/smbiosHelper.h"

#undef WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <batclass.h>
#include <devguid.h>
#include <cfgmgr32.h>
#include <winternl.h>

static const char* detectWithCmApi(FFBatteryOptions* options, FFlist* results)
{
    //https://learn.microsoft.com/en-us/windows-hardware/drivers/install/using-device-interfaces
    ULONG cchDeviceInterfaces = 0;
    CONFIGRET cr = CM_Get_Device_Interface_List_SizeW(&cchDeviceInterfaces, (LPGUID)&GUID_DEVCLASS_BATTERY, NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
    if (cr != CR_SUCCESS)
        return "CM_Get_Device_Interface_List_SizeW() failed";

    if (cchDeviceInterfaces <= 1)
        return NULL; // Not found

    wchar_t* FF_AUTO_FREE mszDeviceInterfaces = (wchar_t*)malloc(cchDeviceInterfaces * sizeof(wchar_t));
    cr = CM_Get_Device_Interface_ListW((LPGUID)&GUID_DEVCLASS_BATTERY, NULL, mszDeviceInterfaces, cchDeviceInterfaces, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
    if (cr != CR_SUCCESS)
        return "CM_Get_Device_Interface_ListW() failed";

    for (const wchar_t* pDeviceInterface = mszDeviceInterfaces; *pDeviceInterface; pDeviceInterface += wcslen(pDeviceInterface) + 1)
    {
        HANDLE FF_AUTO_CLOSE_FD hBattery =
            CreateFileW(pDeviceInterface, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

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
                ffStrbufSetF(&battery->manufactureDate, "%.4d-%.2d-%.2d", date.Year < 1000 ? date.Year + 1900 : date.Year, date.Month, date.Day);
        }

        {
            ffStrbufInit(&battery->serial);
            bqi.InformationLevel = BatterySerialNumber;
            wchar_t name[64];
            if(DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi), name, sizeof(name), &dwOut, NULL))
                ffStrbufSetWS(&battery->serial, name);
        }

        battery->cycleCount = bi.CycleCount;

        battery->temperature = FF_BATTERY_TEMP_UNSET;
        if(options->temp)
        {
            bqi.InformationLevel = BatteryTemperature;
            ULONG temp;
            if(DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi), &temp, sizeof(temp), &dwOut, NULL))
                battery->temperature = temp / 10.0 - 273.15;
        }

        {
            bqi.InformationLevel = BatteryEstimatedTime;
            ULONG time;
            if(DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi), &time, sizeof(time), &dwOut, NULL))
                battery->timeRemaining = time == BATTERY_UNKNOWN_TIME ? -1 : (int32_t) time;
        }

        {
            BATTERY_STATUS bs;
            BATTERY_WAIT_STATUS bws = { .BatteryTag = bqi.BatteryTag };
            if(DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_STATUS, &bws, sizeof(bws), &bs, sizeof(bs), &dwOut, NULL) && bs.Capacity != BATTERY_UNKNOWN_CAPACITY && bi.FullChargedCapacity != 0)
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

static const char* detectBySmbios(FFBatteryResult* battery)
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
        battery->timeRemaining = info.EstimatedTime == BATTERY_UNKNOWN_TIME ? -1 : (int32_t) info.EstimatedTime;

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
        ? detectWithCmApi(options, results)
        : detectWithNtApi(options, results);
}
