#include "gpu.h"
#include "common/library.h"
#include "detection/gpu/gpu_driver_specific.h"
#include "util/windows/unicode.h"
#include "util/windows/registry.h"
#include "util/mallocHelper.h"
#include "util/debug.h"
#include "util/windows/nt.h"

#include <cfgmgr32.h>

#define FF_EMPTY_GUID_STR L"{00000000-0000-0000-0000-000000000000}"
enum { FF_GUID_STRLEN = sizeof(FF_EMPTY_GUID_STR) / sizeof(wchar_t) - 1 };

wchar_t regDirectxKey[] = L"SOFTWARE\\Microsoft\\DirectX\\" FF_EMPTY_GUID_STR;
const uint32_t regDirectxKeyPrefixLength = (uint32_t) __builtin_strlen("SOFTWARE\\Microsoft\\DirectX\\");
wchar_t regDriverKey[] = L"SYSTEM\\CurrentControlSet\\Control\\Class\\" FF_EMPTY_GUID_STR L"\\0000";
const uint32_t regDriverKeyPrefixLength = (uint32_t) __builtin_strlen("SYSTEM\\CurrentControlSet\\Control\\Class\\");

#define GUID_DEVCLASS_DISPLAY_STRING L"{4d36e968-e325-11ce-bfc1-08002be10318}" // Found in <devguid.h>

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    FF_DEBUG("Starting GPU detection");

    ULONG devIdListSize = 0;
    if (CM_Get_Device_ID_List_SizeW(&devIdListSize, GUID_DEVCLASS_DISPLAY_STRING, CM_GETIDLIST_FILTER_CLASS | CM_GETIDLIST_FILTER_PRESENT) != CR_SUCCESS || devIdListSize <= 1)
    {
        FF_DEBUG("No display devices found, list size: %lu", devIdListSize);
        return "No display devices found";
    }

    FF_DEBUG("Found device ID list size: %lu", devIdListSize);

    FF_AUTO_FREE DEVINSTID_W devIdList = malloc(devIdListSize * sizeof(*devIdList));

    if (CM_Get_Device_ID_ListW(GUID_DEVCLASS_DISPLAY_STRING, devIdList, devIdListSize, CM_GETIDLIST_FILTER_CLASS | CM_GETIDLIST_FILTER_PRESENT) != CR_SUCCESS)
    {
        FF_DEBUG("CM_Get_Device_ID_ListW failed");
        return "CM_Get_Device_ID_ListW failed";
    }

    FF_MAYBE_UNUSED int deviceCount = 0;
    for (wchar_t* devId = devIdList; *devId; devId += wcslen(devId) + 1)
    {
        FF_DEBUG("Processing device ID: %ls", devId);

        DEVINST devInst = 0;

        {
            if (CM_Locate_DevNodeW(&devInst, devId, CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS)
            {
                FF_DEBUG("Failed to get device instance ID or locate device node");
                continue;
            }
            FF_DEBUG("Device instance ID: %lu", devInst);
        }

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        deviceCount++;
        FF_DEBUG("Added GPU #%d to list", deviceCount);

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

        unsigned vendorId = 0, deviceId = 0, subSystemId = 0, revId = 0;
        if (swscanf(devId, L"PCI\\VEN_%x&DEV_%x&SUBSYS_%x&REV_%x", &vendorId, &deviceId, &subSystemId, &revId) == 4)
        {
            FF_DEBUG("Parsed PCI IDs - Vendor: 0x%x, Device: 0x%x, SubSystem: 0x%x, Rev: 0x%x", vendorId, deviceId, subSystemId, revId);
            ffStrbufSetStatic(&gpu->vendor, ffGPUGetVendorString(vendorId));
        }
        else
        {
            FF_DEBUG("Failed to parse PCI device information from instance ID");
        }

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
                FF_DEBUG("PCI location - Bus: %u, Device: %u, Function: %u, DeviceID: %llu", pciBus, pciDev, pciFunc, gpu->deviceId);
            }
            else
            {
                FF_DEBUG("Failed to get PCI address");
            }
        }
        else
        {
            FF_DEBUG("Failed to get PCI bus number");
        }

        uint64_t adapterLuid = 0;

        FF_HKEY_AUTO_DESTROY hVideoIdKey = NULL;

        wchar_t buffer[256];
        ULONG bufferLen = 0;

        FF_DEBUG("Get device description as device name");
        bufferLen = sizeof(buffer);
        if (CM_Get_DevNode_Registry_PropertyW(devInst, CM_DRP_DEVICEDESC, NULL, buffer, &bufferLen, 0) == CR_SUCCESS)
        {
            ffStrbufSetWS(&gpu->name, buffer);
            FF_DEBUG("Found device description: %s", gpu->name.chars);
        }
        else
        {
            FF_DEBUG("Failed to get device description");
        }

        if (wcsncmp(devId, L"SWD\\", 4) == 0 || wcsncmp(devId, L"ROOT\\DISPLAY\\", 13) == 0)
        {
            FF_DEBUG("Skipping virtual devices to avoid duplicates");
            continue;
        }

        if (CM_Open_DevNode_Key(devInst, KEY_QUERY_VALUE, 0, RegDisposition_OpenExisting, &hVideoIdKey, CM_REGISTRY_HARDWARE) == CR_SUCCESS)
        {
            FF_DEBUG("Opened device node registry key");
            bufferLen = sizeof(buffer);
            if (RegGetValueW(hVideoIdKey, NULL, L"VideoID", RRF_RT_REG_SZ, NULL, buffer, &bufferLen) == ERROR_SUCCESS &&
                bufferLen == (FF_GUID_STRLEN + 1) * sizeof(wchar_t))
            {
                FF_DEBUG("Found VideoID: %ls", buffer);
                wmemcpy(regDirectxKey + regDirectxKeyPrefixLength, buffer, FF_GUID_STRLEN);
                FF_HKEY_AUTO_DESTROY hDirectxKey = NULL;
                if (ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regDirectxKey, &hDirectxKey, NULL))
                {
                    FF_DEBUG("Opened DirectX registry key");

                    if (gpu->vendor.length == 0)
                    {
                        uint32_t vendorId = 0;
                        if(ffRegReadUint(hDirectxKey, L"VendorId", &vendorId, NULL) && vendorId)
                        {
                            FF_DEBUG("Found vendor ID from DirectX registry: 0x%x", vendorId);
                            ffStrbufSetStatic(&gpu->vendor, ffGPUGetVendorString(vendorId));
                        }
                    }

                    if (gpu->name.length == 0)
                    {
                        FF_DEBUG("Trying to get GPU name from DirectX registry");
                        if (ffRegReadStrbuf(hDirectxKey, L"Description", &gpu->name, NULL))
                            FF_DEBUG("Found GPU description: %s", gpu->name.chars);
                    }

                    if (ffRegReadUint64(hDirectxKey, L"DedicatedVideoMemory", &gpu->dedicated.total, NULL))
                        FF_DEBUG("Found dedicated video memory: %llu bytes", gpu->dedicated.total);

                    if (ffRegReadUint64(hDirectxKey, L"DedicatedSystemMemory", &gpu->shared.total, NULL))
                    {
                        FF_DEBUG("Found dedicated system memory: %llu bytes", gpu->shared.total);
                        uint64_t sharedSystemMemory = 0;
                        if (ffRegReadUint64(hDirectxKey, L"SharedSystemMemory", &sharedSystemMemory, NULL))
                        {
                            gpu->shared.total += sharedSystemMemory;
                            FF_DEBUG("Added shared system memory: %llu bytes, total shared: %llu bytes", sharedSystemMemory, gpu->shared.total);
                        }
                    }

                    if (ffRegReadUint64(hDirectxKey, L"AdapterLuid", &adapterLuid, NULL))
                    {
                        FF_DEBUG("Found adapter LUID: %llu", adapterLuid);
                        if (!gpu->deviceId) gpu->deviceId = adapterLuid;
                    }

                    uint32_t featureLevel = 0;
                    if(ffRegReadUint(hDirectxKey, L"MaxD3D12FeatureLevel", &featureLevel, NULL) && featureLevel)
                    {
                        FF_DEBUG("Found D3D12 feature level: 0x%x", featureLevel);
                        ffStrbufSetF(&gpu->platformApi, "Direct3D 12.%u", (featureLevel & 0x0F00) >> 8);
                    }
                    else if(ffRegReadUint(hDirectxKey, L"MaxD3D11FeatureLevel", &featureLevel, NULL) && featureLevel)
                    {
                        FF_DEBUG("Found D3D11 feature level: 0x%x", featureLevel);
                        ffStrbufSetF(&gpu->platformApi, "Direct3D 11.%u", (featureLevel & 0x0F00) >> 8);
                    }

                    uint64_t driverVersion = 0;
                    if(ffRegReadUint64(hDirectxKey, L"DriverVersion", &driverVersion, NULL) && driverVersion)
                    {
                        FF_DEBUG("Found driver version: %llu", driverVersion);
                        ffStrbufSetF(&gpu->driver, "%u.%u.%u.%u",
                            (unsigned) (driverVersion >> 48) & 0xFFFF,
                            (unsigned) (driverVersion >> 32) & 0xFFFF,
                            (unsigned) (driverVersion >> 16) & 0xFFFF,
                            (unsigned) (driverVersion >> 0) & 0xFFFF
                        );
                    }
                }
                else
                {
                    FF_DEBUG("Failed to open DirectX registry key");
                }
            }
            else
            {
                FF_DEBUG("Failed to get VideoID or invalid buffer length");
            }
        }
        else
        {
            FF_DEBUG("Failed to open device node registry key");
        }

        if (gpu->vendor.length == 0 || gpu->name.length == 0)
        {
            FF_DEBUG("Trying fallback registry method for vendor/name");
            bufferLen = sizeof(buffer);
            if (CM_Get_DevNode_Registry_PropertyW(devInst, CM_DRP_DRIVER, NULL, buffer, &bufferLen, 0) == CR_SUCCESS &&
                bufferLen == (FF_GUID_STRLEN + strlen("\\0000") + 1) * 2)
            {
                FF_DEBUG("Found driver GUID: %ls", buffer);
                wmemcpy(regDriverKey + regDriverKeyPrefixLength, buffer, FF_GUID_STRLEN + strlen("\\0000"));
                FF_HKEY_AUTO_DESTROY hRegDriverKey = NULL;
                if (ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regDriverKey, &hRegDriverKey, NULL))
                {
                    FF_DEBUG("Opened driver registry key");

                    if (gpu->vendor.length == 0 && ffRegReadStrbuf(hRegDriverKey, L"ProviderName", &gpu->vendor, NULL))
                    {
                        FF_DEBUG("Found provider name: %s", gpu->vendor.chars);
                        if (ffStrbufContainS(&gpu->vendor, "Intel"))
                            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
                        else if (ffStrbufContainS(&gpu->vendor, "NVIDIA"))
                            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);
                        else if (ffStrbufContainS(&gpu->vendor, "AMD") || ffStrbufContainS(&gpu->vendor, "ATI"))
                            ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
                    }
                    if (gpu->name.length == 0 && ffRegReadStrbuf(hRegDriverKey, L"DriverDesc", &gpu->name, NULL))
                        FF_DEBUG("Found driver description: %s", gpu->name.chars);
                    if (gpu->driver.length == 0 && ffRegReadStrbuf(hRegDriverKey, L"DriverVersion", &gpu->driver, NULL))
                        FF_DEBUG("Found driver version: %s", gpu->driver.chars);
                    if (gpu->dedicated.total == FF_GPU_VMEM_SIZE_UNSET)
                    {
                        if (!ffRegReadUint64(hRegDriverKey, L"HardwareInformation.qwMemorySize", &gpu->dedicated.total, NULL))
                        {
                            uint32_t memorySize = 0;
                            if (ffRegReadUint(hRegDriverKey, L"HardwareInformation.MemorySize", &memorySize, NULL))
                            {
                                gpu->dedicated.total = memorySize;
                                FF_DEBUG("Found memory size from hardware info: %u bytes", memorySize);
                            }
                        }
                        else
                        {
                            FF_DEBUG("Found qwMemorySize from hardware info: %llu bytes", gpu->dedicated.total);
                        }
                    }
                }
                else
                {
                    FF_DEBUG("Failed to open driver registry key");
                }
            }
            else
            {
                FF_DEBUG("Failed to get driver GUID or invalid buffer length");
            }
        }

        __typeof__(&ffDetectNvidiaGpuInfo) detectFn;
        const char* dllName;

        if (options->driverSpecific && getDriverSpecificDetectionFn(gpu->vendor.chars, &detectFn, &dllName))
        {
            FF_DEBUG("Calling driver-specific detection function for vendor: %s, DLL: %s", gpu->vendor.chars, dllName);
            detectFn(
                &(FFGpuDriverCondition) {
                    .type = (deviceId > 0 ? FF_GPU_DRIVER_CONDITION_TYPE_DEVICE_ID : 0)
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
            FF_DEBUG("Driver-specific detection completed");
        }
        else if (options->driverSpecific)
        {
            FF_DEBUG("No driver-specific detection function found for vendor: %s", gpu->vendor.chars);
        }

        if (gpu->type == FF_GPU_TYPE_UNKNOWN && adapterLuid > 0)
        {
            FF_DEBUG("Trying to determine GPU type using D3DKMT APIs");
            HMODULE hgdi32 = GetModuleHandleW(L"gdi32.dll");
            if (hgdi32)
            {
                FF_LIBRARY_LOAD_SYMBOL_LAZY(hgdi32, D3DKMTOpenAdapterFromLuid);
                if (ffD3DKMTOpenAdapterFromLuid) // Windows 8 and later
                {
                    D3DKMT_OPENADAPTERFROMLUID openAdapterFromLuid = { .AdapterLuid = *(LUID*)&adapterLuid };
                    if (NT_SUCCESS(ffD3DKMTOpenAdapterFromLuid(&openAdapterFromLuid)))
                    {
                        FF_DEBUG("Successfully opened adapter from LUID");

                        D3DKMT_ADAPTERTYPE adapterType = {};
                        D3DKMT_QUERYADAPTERINFO queryAdapterInfo = {
                            .hAdapter = openAdapterFromLuid.hAdapter,
                            .Type = KMTQAITYPE_ADAPTERTYPE,
                            .pPrivateDriverData = &adapterType,
                            .PrivateDriverDataSize = sizeof(adapterType),
                        };
                        if (NT_SUCCESS(D3DKMTQueryAdapterInfo(&queryAdapterInfo))) // Vista and later
                        {
                            FF_DEBUG("Queried adapter type - HybridDiscrete: %d, HybridIntegrated: %d", adapterType.HybridDiscrete, adapterType.HybridIntegrated);
                            if (adapterType.HybridDiscrete)
                                gpu->type = FF_GPU_TYPE_DISCRETE;
                            else if (adapterType.HybridIntegrated)
                                gpu->type = FF_GPU_TYPE_INTEGRATED;
                        }
                        else
                        {
                            FF_DEBUG("Failed to query adapter type");
                        }

                        if (gpu->frequency == FF_GPU_FREQUENCY_UNSET)
                        {
                            FF_DEBUG("Trying to get GPU frequency information");
                            for (ULONG nodeIdx = 0; ; nodeIdx++)
                            {
                                D3DKMT_NODEMETADATA nodeMetadata = {
                                    .NodeOrdinalAndAdapterIndex = (0 << 16) | nodeIdx,
                                };
                                queryAdapterInfo = (D3DKMT_QUERYADAPTERINFO) {
                                    .hAdapter = openAdapterFromLuid.hAdapter,
                                    .Type = KMTQAITYPE_NODEMETADATA,
                                    .pPrivateDriverData = &nodeMetadata,
                                    .PrivateDriverDataSize = sizeof(nodeMetadata),
                                };
                                if (!NT_SUCCESS(D3DKMTQueryAdapterInfo(&queryAdapterInfo)))
                                {
                                    FF_DEBUG("No more nodes to query (index %lu)", nodeIdx);
                                    break; // Windows 10 and later
                                }
                                if (nodeMetadata.NodeData.EngineType != DXGK_ENGINE_TYPE_3D)
                                {
                                    FF_DEBUG("Skipping node %lu (not 3D engine)", nodeIdx);
                                    continue;
                                }

                                D3DKMT_QUERYSTATISTICS queryStatistics = {
                                    .Type = D3DKMT_QUERYSTATISTICS_NODE2,
                                    .AdapterLuid = *(LUID*)&adapterLuid,
                                    .QueryNode2 = { .PhysicalAdapterIndex = 0, .NodeOrdinal = (UINT16) nodeIdx },
                                };
                                if (NT_SUCCESS(D3DKMTQueryStatistics(&queryStatistics))) // Windows 11 (22H2) and later
                                {
                                    gpu->frequency = (uint32_t) (queryStatistics.QueryResult.NodeInformation.NodePerfData.MaxFrequency / 1000 / 1000);
                                    FF_DEBUG("Found GPU frequency: %u MHz", gpu->frequency);
                                    break;
                                }
                                else
                                {
                                    FF_DEBUG("Failed to query node statistics for node %lu", nodeIdx);
                                }
                            }
                        }

                        D3DKMT_CLOSEADAPTER closeAdapter = { .hAdapter = openAdapterFromLuid.hAdapter };
                        (void) D3DKMTCloseAdapter(&closeAdapter);
                        openAdapterFromLuid.hAdapter = 0;
                        FF_DEBUG("Closed adapter handle");
                    }
                    else
                    {
                        FF_DEBUG("Failed to open adapter from LUID");
                    }

                    if (options->temp && gpu->temperature == FF_GPU_TEMP_UNSET)
                    {
                        FF_DEBUG("Trying to get GPU temperature");
                        D3DKMT_QUERYSTATISTICS queryStatistics = {
                            .Type = D3DKMT_QUERYSTATISTICS_PHYSICAL_ADAPTER,
                            .AdapterLuid = *(LUID*)&adapterLuid,
                            .QueryPhysAdapter = { .PhysicalAdapterIndex = 0 },
                        };
                        if (NT_SUCCESS(D3DKMTQueryStatistics(&queryStatistics)) &&
                            queryStatistics.QueryResult.PhysAdapterInformation.AdapterPerfData.Temperature != 0)
                        {
                            gpu->temperature = queryStatistics.QueryResult.PhysAdapterInformation.AdapterPerfData.Temperature / 10.0;
                            FF_DEBUG("Found GPU temperature: %.1f°C", gpu->temperature);
                        }
                        else
                        {
                            FF_DEBUG("Failed to get GPU temperature or temperature is 0");
                        }
                    }
                }
                else
                {
                    FF_DEBUG("D3DKMTOpenAdapterFromLuid not available");
                }
            }
            else
            {
                FF_DEBUG("Failed to get gdi32.dll module handle");
            }
        }

        if (gpu->type == FF_GPU_TYPE_UNKNOWN)
        {
            FF_DEBUG("Using fallback GPU type detection");
            if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_INTEL)
            {
                gpu->type = gpu->deviceId == 20 ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;
                FF_DEBUG("Intel GPU type determined: %s", gpu->type == FF_GPU_TYPE_INTEGRATED ? "Integrated" : "Discrete");
            }
            else if (gpu->dedicated.total != FF_GPU_VMEM_SIZE_UNSET)
            {
                gpu->type = gpu->dedicated.total >= 1024 * 1024 * 1024 ? FF_GPU_TYPE_DISCRETE : FF_GPU_TYPE_INTEGRATED;
                FF_DEBUG("GPU type determined by memory size (%llu bytes): %s", gpu->dedicated.total, gpu->type == FF_GPU_TYPE_DISCRETE ? "Discrete" : "Integrated");
            }
            else
            {
                FF_DEBUG("Unable to determine GPU type");
            }
        }

        FF_DEBUG("Completed processing GPU #%d - Vendor: %s, Name: %s, Type: %d", deviceCount, gpu->vendor.chars, gpu->name.chars, gpu->type);
    }

    FF_DEBUG("GPU detection completed, found %d devices", deviceCount);
    return NULL;
}
