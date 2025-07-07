#include "gpu.h"
#include "common/library.h"
#include "detection/gpu/gpu_driver_specific.h"
#include "util/windows/unicode.h"
#include "util/windows/registry.h"
#include "util/mallocHelper.h"

#define INITGUID
#include <cfgmgr32.h>
#include <ntddvdeo.h>
#include <devpkey.h>
#include "util/windows/nt.h"

#define FF_EMPTY_GUID_STR L"{00000000-0000-0000-0000-000000000000}"
enum { FF_GUID_STRLEN = sizeof(FF_EMPTY_GUID_STR) / sizeof(wchar_t) - 1 };

wchar_t regDirectxKey[] = L"SOFTWARE\\Microsoft\\DirectX\\" FF_EMPTY_GUID_STR;
const uint32_t regDirectxKeyPrefixLength = (uint32_t) __builtin_strlen("SOFTWARE\\Microsoft\\DirectX\\");
wchar_t regDriverKey[] = L"SYSTEM\\CurrentControlSet\\Control\\Class\\" FF_EMPTY_GUID_STR L"\\0000";
const uint32_t regDriverKeyPrefixLength = (uint32_t) __builtin_strlen("SYSTEM\\CurrentControlSet\\Control\\Class\\");

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    ULONG devInfListSize = 0;
    if (CM_Get_Device_Interface_List_SizeW(&devInfListSize, (LPGUID)&GUID_DEVINTERFACE_DISPLAY_ADAPTER, NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT) != CR_SUCCESS || devInfListSize <= 1)
        return "No display devices found";

    FF_AUTO_FREE DEVINSTID_W devInfList = malloc(devInfListSize * sizeof(*devInfList));

    if (CM_Get_Device_Interface_ListW((LPGUID)&GUID_DEVINTERFACE_DISPLAY_ADAPTER, NULL, devInfList, devInfListSize, CM_GET_DEVICE_INTERFACE_LIST_PRESENT) != CR_SUCCESS)
        return "CM_Get_Device_Interface_ListW failed";

    for (wchar_t* devInf = devInfList; *devInf; devInf += wcslen(devInf) + 1)
    {
        if (wcsncmp(devInf, L"\\\\?\\ROOT#BasicDisplay#", 22) == 0)
            continue; // Skip Microsoft Basic Display Adapter

        DEVINST devInst = 0;
        wchar_t buffer[256];
        ULONG bufferLen = 0;

        {
            DEVPROPTYPE propertyType;
            bufferLen = sizeof(buffer);
            if (CM_Get_Device_Interface_PropertyW(devInf, &DEVPKEY_Device_InstanceId, &propertyType, (PBYTE) buffer, &bufferLen, 0) != CR_SUCCESS ||
                CM_Locate_DevNodeW(&devInst, buffer, CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS)
                continue;
        }

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInit(&gpu->vendor);
        ffStrbufInit(&gpu->name);
        ffStrbufInit(&gpu->driver);
        ffStrbufInit(&gpu->memoryType);
        ffStrbufInitStatic(&gpu->platformApi, "CM API");
        gpu->index = FF_GPU_INDEX_UNSET;
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = 0;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        uint32_t pciBus = 0, pciAddr = 0, pciDev = 0, pciFunc = 0;

        ULONG pciBufLen = sizeof(pciBus);
        if (CM_Get_DevNode_Registry_PropertyW(devInst, CM_DRP_BUSNUMBER, NULL, &pciBus, &pciBufLen, 0) == CR_SUCCESS)
        {
            pciBufLen = sizeof(pciAddr);
            if (CM_Get_DevNode_Registry_PropertyW(devInst, CM_DRP_ADDRESS, NULL, &pciAddr, &pciBufLen, 0) == CR_SUCCESS)
            {
                pciDev = (pciAddr >> 16) & 0xFFFF;
                pciFunc = pciAddr & 0xFFFF;
                gpu->deviceId = (pciBus * 1000ull) + (pciDev * 10ull) + pciFunc;
                pciAddr = 1; // Set to 1 to indicate that the device is a PCI device
            }
        }

        uint64_t adapterLuid = 0;

        FF_HKEY_AUTO_DESTROY hVideoIdKey = NULL;
        if (CM_Open_DevNode_Key(devInst, KEY_QUERY_VALUE, 0, RegDisposition_OpenExisting, &hVideoIdKey, CM_REGISTRY_HARDWARE) == CR_SUCCESS)
        {
            bufferLen = sizeof(buffer);
            if (RegGetValueW(hVideoIdKey, NULL, L"VideoID", RRF_RT_REG_SZ, NULL, buffer, &bufferLen) == ERROR_SUCCESS &&
                bufferLen == (FF_GUID_STRLEN + 1) * sizeof(wchar_t))
            {
                wmemcpy(regDirectxKey + regDirectxKeyPrefixLength, buffer, FF_GUID_STRLEN);
                FF_HKEY_AUTO_DESTROY hDirectxKey = NULL;
                if (ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regDirectxKey, &hDirectxKey, NULL))
                {
                    uint32_t vendorId = 0;
                    if(ffRegReadUint(hDirectxKey, L"VendorId", &vendorId, NULL) && vendorId)
                        ffStrbufSetStatic(&gpu->vendor, ffGPUGetVendorString(vendorId));

                    if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_INTEL)
                        gpu->type = gpu->deviceId == 20 ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;

                    ffRegReadUint64(hDirectxKey, L"DedicatedVideoMemory", &gpu->dedicated.total, NULL);
                    ffRegReadUint64(hDirectxKey, L"DedicatedSystemMemory", &gpu->shared.total, NULL);

                    if (ffRegReadUint64(hDirectxKey, L"AdapterLuid", &adapterLuid, NULL))
                    {
                        if (!gpu->deviceId) gpu->deviceId = adapterLuid;
                    }

                    uint32_t featureLevel = 0;
                    if(ffRegReadUint(hDirectxKey, L"MaxD3D12FeatureLevel", &featureLevel, NULL) && featureLevel)
                        ffStrbufSetF(&gpu->platformApi, "Direct3D 12.%u", (featureLevel & 0x0F00) >> 8);
                    else if(ffRegReadUint(hDirectxKey, L"MaxD3D11FeatureLevel", &featureLevel, NULL) && featureLevel)
                        ffStrbufSetF(&gpu->platformApi, "Direct3D 11.%u", (featureLevel & 0x0F00) >> 8);

                    uint64_t driverVersion = 0;
                    if(ffRegReadUint64(hDirectxKey, L"DriverVersion", &driverVersion, NULL) && driverVersion)
                    {
                        ffStrbufSetF(&gpu->driver, "%u.%u.%u.%u",
                            (unsigned) (driverVersion >> 48) & 0xFFFF,
                            (unsigned) (driverVersion >> 32) & 0xFFFF,
                            (unsigned) (driverVersion >> 16) & 0xFFFF,
                            (unsigned) (driverVersion >> 0) & 0xFFFF
                        );
                    }
                }
            }
        }

        if (adapterLuid > 0)
        {
            HMODULE hgdi32 = GetModuleHandleW(L"gdi32.dll");
            if (hgdi32)
            {
                FF_LIBRARY_LOAD_SYMBOL_LAZY(hgdi32, D3DKMTOpenAdapterFromLuid);
                if (ffD3DKMTOpenAdapterFromLuid) // Windows 8 and later
                {
                    D3DKMT_OPENADAPTERFROMLUID openAdapterFromLuid = { .AdapterLuid = *(LUID*)&adapterLuid };
                    if (NT_SUCCESS(ffD3DKMTOpenAdapterFromLuid(&openAdapterFromLuid)))
                    {
                        D3DKMT_ADAPTERTYPE adapterType = {};
                        D3DKMT_QUERYADAPTERINFO queryAdapterInfo = {
                            .hAdapter = openAdapterFromLuid.hAdapter,
                            .Type = KMTQAITYPE_ADAPTERTYPE,
                            .pPrivateDriverData = &adapterType,
                            .PrivateDriverDataSize = sizeof(adapterType),
                        };
                        if (NT_SUCCESS(D3DKMTQueryAdapterInfo(&queryAdapterInfo))) // Vista and later
                        {
                            if (adapterType.HybridDiscrete)
                                gpu->type = FF_GPU_TYPE_DISCRETE;
                            else if (adapterType.HybridIntegrated)
                                gpu->type = FF_GPU_TYPE_INTEGRATED;
                        }
                    }
                }
            }
        }

        if (gpu->vendor.length == 0)
        {
            bufferLen = sizeof(buffer);
            if (CM_Get_DevNode_Registry_PropertyW(devInst, CM_DRP_DRIVER, NULL, buffer, &bufferLen, 0) == CR_SUCCESS &&
                bufferLen == (FF_GUID_STRLEN + strlen("\\0000") + 1) * 2)
            {
                wmemcpy(regDriverKey + regDriverKeyPrefixLength, buffer, FF_GUID_STRLEN + strlen("\\0000"));
                FF_HKEY_AUTO_DESTROY hRegDriverKey = NULL;
                if (ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regDriverKey, &hRegDriverKey, NULL))
                {
                    if (ffRegReadStrbuf(hRegDriverKey, L"ProviderName", &gpu->vendor, NULL))
                    {
                        if (ffStrbufContainS(&gpu->vendor, "Intel"))
                            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
                        else if (ffStrbufContainS(&gpu->vendor, "NVIDIA"))
                            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);
                        else if (ffStrbufContainS(&gpu->vendor, "AMD") || ffStrbufContainS(&gpu->vendor, "ATI"))
                            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
                    }
                    if (gpu->driver.length == 0)
                        ffRegReadStrbuf(hRegDriverKey, L"DriverVersion", &gpu->driver, NULL);
                    if (gpu->dedicated.total == FF_GPU_VMEM_SIZE_UNSET)
                    {
                        if (!ffRegReadUint64(hRegDriverKey, L"HardwareInformation.qwMemorySize", &gpu->dedicated.total, NULL))
                        {
                            uint32_t memorySize = 0;
                            if (ffRegReadUint(hRegDriverKey, L"HardwareInformation.MemorySize", &memorySize, NULL))
                                gpu->dedicated.total = memorySize;
                        }
                    }
                }
            }
        }

        __typeof__(&ffDetectNvidiaGpuInfo) detectFn;
        const char* dllName;

        if (getDriverSpecificDetectionFn(gpu->vendor.chars, &detectFn, &dllName) && (options->temp || options->driverSpecific))
        {
            unsigned vendorId = 0, deviceId = 0, subSystemId = 0, revId = 0;
            bufferLen = sizeof(buffer);
            if (CM_Get_DevNode_Registry_PropertyW(devInst, CM_DRP_HARDWAREID, NULL, buffer, &bufferLen, 0) == CR_SUCCESS)
            {
                swscanf(buffer, L"PCI\\VEN_%x&DEV_%x&SUBSYS_%x&REV_%x", &vendorId, &deviceId, &subSystemId, &revId);
                ffStrbufSetStatic(&gpu->vendor, ffGPUGetVendorString(vendorId));
            }

            detectFn(
                &(FFGpuDriverCondition) {
                    .type = FF_GPU_DRIVER_CONDITION_TYPE_DEVICE_ID
                            | (adapterLuid > 0 ? FF_GPU_DRIVER_CONDITION_TYPE_LUID : 0)
                            | (pciAddr > 0 ? FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID : 0),
                    .pciDeviceId = {
                        .deviceId = deviceId,
                        .vendorId = vendorId,
                        .subSystemId = subSystemId,
                        .revId = revId,
                    },
                    .pciBusId = {
                        .domain = 0,
                        .bus = pciBus,
                        .device = pciDev,
                        .func = pciFunc,
                    },
                    .luid = adapterLuid,
                },
                (FFGpuDriverResult){
                    .index = &gpu->index,
                    .temp = options->temp ? &gpu->temperature : NULL,
                    .memory = options->driverSpecific ? &gpu->dedicated : NULL,
                    .sharedMemory = options->driverSpecific ? &gpu->shared : NULL,
                    .memoryType = options->driverSpecific ? &gpu->memoryType : NULL,
                    .coreCount = options->driverSpecific ? (uint32_t*) &gpu->coreCount : NULL,
                    .coreUsage = options->driverSpecific ? &gpu->coreUsage : NULL,
                    .type = &gpu->type,
                    .name = &gpu->name,
                    .frequency = options->driverSpecific ? &gpu->frequency : NULL,
                },
                dllName
            );
        }

        if (!gpu->name.length)
        {
            bufferLen = sizeof(buffer);
            if (CM_Get_DevNode_Registry_PropertyW(devInst, CM_DRP_DEVICEDESC, NULL, buffer, &bufferLen, 0) == CR_SUCCESS)
                ffStrbufSetWS(&gpu->name, buffer);
        }
        if (gpu->type == FF_GPU_TYPE_UNKNOWN && gpu->dedicated.total != FF_GPU_VMEM_SIZE_UNSET)
            gpu->type = gpu->dedicated.total >= 1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;
    }

    return NULL;
}
