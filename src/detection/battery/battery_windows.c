#include "battery.h"
#include "util/windows/unicode.h"
#include "util/mallocHelper.h"

#undef WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <setupapi.h>
#include <batclass.h>
#include <devguid.h>
#include <winternl.h>

#ifdef FF_USE_WIN_NTAPI
    NTSYSCALLAPI
    NTSTATUS
    NTAPI
    NtPowerInformation(
        IN POWER_INFORMATION_LEVEL InformationLevel,
        IN PVOID InputBuffer OPTIONAL,
        IN ULONG InputBufferLength,
        OUT PVOID OutputBuffer OPTIONAL,
        IN ULONG OutputBufferLength);
    #define CallNtPowerInformation NtPowerInformation
#endif

static inline void wrapCloseHandle(HANDLE* handle)
{
    if(*handle)
        CloseHandle(*handle);
}
static inline void wrapSetupDiDestroyDeviceInfoList(HDEVINFO* hdev)
{
    if(*hdev)
        SetupDiDestroyDeviceInfoList(*hdev);
}

const char* ffDetectBatteryImpl(FFinstance* instance, FFlist* results)
{
    if(instance->config.allowSlowOperations)
    {
        //https://learn.microsoft.com/en-us/windows/win32/power/enumerating-battery-devices
        HDEVINFO hdev __attribute__((__cleanup__(wrapSetupDiDestroyDeviceInfoList))) =
            SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        if(hdev == INVALID_HANDLE_VALUE)
            return "SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY) failed";

        for(DWORD idev = 0;; idev++)
        {
            SP_DEVICE_INTERFACE_DATA did = { .cbSize = sizeof(did) };
            if(!SetupDiEnumDeviceInterfaces(hdev, NULL, &GUID_DEVCLASS_BATTERY, idev, &did))
                break;

            DWORD cbRequired = 0;
            SetupDiGetDeviceInterfaceDetailW(hdev, &did, NULL, 0, &cbRequired, NULL); //Fail with not enough buffer
            SP_DEVICE_INTERFACE_DETAIL_DATA_W* FF_AUTO_FREE pdidd = (SP_DEVICE_INTERFACE_DETAIL_DATA_W*)malloc(cbRequired);
            if(!pdidd)
                break; //Out of memory

            pdidd->cbSize = sizeof(*pdidd);
            if(!SetupDiGetDeviceInterfaceDetailW(hdev, &did, pdidd, cbRequired, &cbRequired, NULL))
                continue;

            HANDLE __attribute__((__cleanup__(wrapCloseHandle))) hBattery =
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

            BatteryResult* battery = (BatteryResult*)ffListAdd(results);

            if(memcmp(bi.Chemistry, "PbAc", 4) == 0)
                ffStrbufInitS(&battery->technology, "Lead Acid");
            else if(memcmp(bi.Chemistry, "LION", 4) == 0 || memcmp(bi.Chemistry, "Li-I", 4) == 0)
                ffStrbufInitS(&battery->technology, "Lithium Ion");
            else if(memcmp(bi.Chemistry, "NiCd", 4) == 0)
                ffStrbufInitS(&battery->technology, "Nickel Cadmium");
            else if(memcmp(bi.Chemistry, "NiMH", 4) == 0)
                ffStrbufInitS(&battery->technology, "Nickel Metal Hydride");
            else if(memcmp(bi.Chemistry, "NiZn", 4) == 0)
                ffStrbufInitS(&battery->technology, "Nickel Zinc");
            else if(memcmp(bi.Chemistry, "RAM\0", 4) == 0)
                ffStrbufInitS(&battery->technology, "Rechargeable Alkaline-Manganese");
            else
                ffStrbufInitS(&battery->technology, "Unknown");

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

            battery->temperature = 0.0/0.0;
            if(instance->config.batteryTemp)
            {
                bqi.InformationLevel = BatteryTemperature;
                ULONG temp;
                if(DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION, &bqi, sizeof(bqi), &temp, sizeof(temp), &dwOut, NULL))
                    battery->temperature = temp;
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
                    ffStrbufAppendS(&battery->status, "Charging");
                if(bs.PowerState & BATTERY_CRITICAL)
                    ffStrbufAppendS(&battery->status, "Critical, ");
                ffStrbufTrimRight(&battery->status, ' ');
                ffStrbufTrimRight(&battery->status, ',');
            }
        }
    }
    else
    {
        SYSTEM_BATTERY_STATE info;
        if (NT_SUCCESS(CallNtPowerInformation(SystemBatteryState, NULL, 0, &info, sizeof(info))) && info.BatteryPresent)
        {
            BatteryResult* battery = (BatteryResult*)ffListAdd(results);
            ffStrbufInit(&battery->modelName);
            ffStrbufInit(&battery->manufacturer);
            ffStrbufInit(&battery->technology);
            ffStrbufInit(&battery->status);
            battery->temperature = 0.0/0.0;

            battery->capacity = info.RemainingCapacity * 100.0 / info.MaxCapacity;
            if(info.AcOnLine)
            {
                ffStrbufAppendS(&battery->status, "AC Connected");
                if(info.Charging)
                    ffStrbufAppendS(&battery->status, ", Charging");
            }
            else if(info.Discharging)
                ffStrbufAppendS(&battery->status, "Discharging");
        }
    }

    return NULL;
}
