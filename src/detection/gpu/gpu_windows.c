#include "detection/gpu/gpu.h"
#if __linux__
    #define FF_GPU_DRIVER_DLLNAME_PATH_PREFIX "/usr/lib/wsl/lib/"
#endif
#include "detection/gpu/gpu_driver_specific.h"
#include "common/debug.h"

#include <inttypes.h>
#include "d3dkmthk.h"

#if _WIN32
    #include "common/windows/unicode.h"
    #include "common/windows/registry.h"
    #include "common/mallocHelper.h"

    #define INITGUID
    #include <windows.h>
    #include <cfgmgr32.h>
    #include <devguid.h>
    #include <devpkey.h>
    #include <pciprop.h>

    #define GUID_DEVCLASS_DISPLAY_STRING L"{4d36e968-e325-11ce-bfc1-08002be10318}" // Found in <devguid.h>

// https://wine-devel.winehq.narkive.com/vKO2Bkgj/patch-1-2-dxgi-tests-add-test-for-enumerating-display-adapters-using-setupapi
DEFINE_DEVPROPKEY(DEVPROPKEY_DISPLAY_ADAPTER_LUID, 0x60b193cb, 0x5276, 0x4d0f, 0x96, 0xfc, 0xf1, 0x73, 0xab, 0xad, 0x3e, 0xc6, 2);

static bool queryDeviceInfoCM(FFGPUResult* gpu, uint64_t luid, D3DKMT_DEVICE_IDS* outDeviceIds, bool queryPcieGen) {
    FF_DEBUG("Query PCI device info: %08llX with LUID: %08llX", gpu->deviceId, luid);

    static FFlist deviceIdsCache;
    static bool initialized;
    typedef struct {
        uint32_t currentLinkSpeed;
        uint32_t currentLinkWidth;
        uint32_t maxLinkSpeed;
        uint32_t maxLinkWidth;
        D3DKMT_DEVICE_IDS deviceIds;
        uint64_t pciAddr;
        uint64_t luid;
        FFstrbuf vendor;
        FFstrbuf name;
    } CacheEntry;

    if (!initialized) {
        initialized = true;
        ffListInit(&deviceIdsCache);

        ULONG devIdListSize = 0;
        if (CM_Get_Device_ID_List_SizeW(&devIdListSize, GUID_DEVCLASS_DISPLAY_STRING, CM_GETIDLIST_FILTER_CLASS | CM_GETIDLIST_FILTER_PRESENT) != CR_SUCCESS || devIdListSize <= 1) {
            FF_DEBUG("No display devices found, list size: %lu", devIdListSize);
            return false;
        }

        FF_DEBUG("Found device ID list size: %lu", devIdListSize);

        FF_AUTO_FREE DEVINSTID_W devIdList = malloc(devIdListSize * sizeof(*devIdList));

        if (CM_Get_Device_ID_ListW(GUID_DEVCLASS_DISPLAY_STRING, devIdList, devIdListSize, CM_GETIDLIST_FILTER_CLASS | CM_GETIDLIST_FILTER_PRESENT) != CR_SUCCESS) {
            FF_DEBUG("CM_Get_Device_ID_ListW failed");
            return false;
        }

        for (wchar_t* devId = devIdList; *devId; devId += wcslen(devId) + 1) {
            FF_DEBUG("Processing device ID: %ls", devId);

            DEVINST devInst = 0;

            if (CM_Locate_DevNodeW(&devInst, devId, CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS) {
                FF_DEBUG("Failed to get device instance ID or locate device node");
                continue;
            }
            FF_DEBUG("Device instance ID: %lu", devInst);

            for (wchar_t* p = devId; *p; p++) {
                if (*p >= L'a' && *p <= L'z') {
                    *p -= L'a' - L'A';
                }
            }

            CacheEntry* entry = FF_LIST_ADD(CacheEntry, deviceIdsCache);
            *entry = (CacheEntry){};

            ULONG bufLen = sizeof(entry->luid);
            DEVPROPTYPE type;
            CONFIGRET ret = CM_Get_DevNode_PropertyW(devInst, &DEVPROPKEY_DISPLAY_ADAPTER_LUID, &type, (PBYTE) &entry->luid, &bufLen, 0);
            if (ret != CR_SUCCESS) { // Not available on Windows 8.1
                FF_DEBUG("Failed to get device LUID: %s", ffDebugConfigRet(ret));

                uint32_t pciBus = 0;
                ULONG pciBufLen = sizeof(pciBus);
                if (CM_Get_DevNode_Registry_PropertyW(devInst, CM_DRP_BUSNUMBER, nullptr, &pciBus, &pciBufLen, 0) == CR_SUCCESS) {
                    uint32_t pciAddr = 0;
                    pciBufLen = sizeof(pciAddr);
                    if (CM_Get_DevNode_Registry_PropertyW(devInst, CM_DRP_ADDRESS, nullptr, &pciAddr, &pciBufLen, 0) == CR_SUCCESS) {
                        entry->pciAddr = ffGPUPciAddr2Id(0, pciBus, (pciAddr >> 16) & 0xFFFF, pciAddr & 0xFFFF);
                        FF_DEBUG("Cached device IDs for PCI bus %u: vendor=0x%04x device=0x%04x", pciBus, entry->deviceIds.VendorID, entry->deviceIds.DeviceID);
                    } else {
                        FF_DEBUG("Failed to get PCI address");
                    }
                } else {
                    FF_DEBUG("Failed to get PCI bus number");
                }

                if (entry->pciAddr == 0) {
                    FF_DEBUG("Skipping device ID: %ls due to missing LUID and PCI address", devId);
                    deviceIdsCache.length--;
                    continue;
                }
            }

            // L"PCI\\VEN_10DE&DEV_2782&SUBSYS_513417AA&REV_A1\\4&3674a6b9&0&0008"
            if (swscanf(devId + 4, L"VEN_%x&DEV_%x&SUBSYS_%4x%4x&REV_%x", &entry->deviceIds.VendorID, &entry->deviceIds.DeviceID, &entry->deviceIds.SubSystemID, &entry->deviceIds.SubVendorID, &entry->deviceIds.RevisionID) >= 2) {
                FF_DEBUG("Parsed PCI IDs - Vendor: 0x%04x, Device: 0x%04x, SubVendor: 0x%04x, SubSystem: 0x%04x, Rev: 0x%04x", entry->deviceIds.VendorID, entry->deviceIds.DeviceID, entry->deviceIds.SubVendorID, entry->deviceIds.SubSystemID, entry->deviceIds.RevisionID);
                // I thought it was DXGKMDT_OPM_BUS_TYPE_PCI, but it turns out to be false
                // Who TF knows what 1 actually means. It's just reported by most graphic cards
                // And yeah, DXGKMDT_OPM_BUS_TYPE_PCIEXPRESS (3) exists
                entry->deviceIds.BusType = 1;
            } else {
                FF_DEBUG("Failed to parse PCI IDs from device ID: %ls", devId);
                entry->deviceIds.VendorID = -1u;
            }

            wchar_t wstr[256];
            bufLen = sizeof(wstr);
            ret = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_FriendlyName, &type, (PBYTE) wstr, &bufLen, 0);
            if (ret == CR_SUCCESS) {
                ffStrbufSetNWS(&entry->name, (bufLen - 1) / sizeof(wchar_t), wstr);
                FF_DEBUG("Device friendly name: %ls", wstr);
            } else {
                FF_DEBUG("Failed to get device friendly name: %s", ffDebugConfigRet(ret));

                bufLen = sizeof(wstr);
                ret = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_DeviceDesc, &type, (PBYTE) wstr, &bufLen, 0);
                if (ret == CR_SUCCESS) {
                    ffStrbufSetNWS(&entry->name, (bufLen - 1) / sizeof(wchar_t), wstr);
                    FF_DEBUG("Device description: %ls", wstr);
                } else {
                    FF_DEBUG("Failed to get device description: %s", ffDebugConfigRet(ret));
                }
            }

            bufLen = sizeof(wstr);
            ret = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_Manufacturer, &type, (PBYTE) wstr, &bufLen, 0);
            if (ret == CR_SUCCESS) {
                ffStrbufSetNWS(&entry->vendor, (bufLen - 1) / sizeof(wchar_t), wstr);
                FF_DEBUG("Device vendor name: %ls", wstr);
            } else {
                FF_DEBUG("Failed to get device vendor name: %s", ffDebugConfigRet(ret));
            }

            if (queryPcieGen && entry->deviceIds.VendorID != -1u) {
                DEVPROPTYPE propType;

                bufLen = sizeof(entry->maxLinkSpeed);
                // Reports PCIe gen despite the PKEY name
                CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_PciDevice_MaxLinkSpeed, &propType, (PBYTE) &entry->maxLinkSpeed, &bufLen, 0);
                if (ret == CR_SUCCESS) {
                    FF_DEBUG("PCIe max GEN: %u", entry->maxLinkSpeed);
                } else {
                    FF_DEBUG("Failed to get PCIe max GEN: %s", ffDebugConfigRet(ret));
                }

                if (entry->maxLinkSpeed != FF_GPU_PCIE_SPEED_UNSET) {
                    bufLen = sizeof(entry->maxLinkWidth);
                    ret = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_PciDevice_MaxLinkWidth, &propType, (PBYTE) &entry->maxLinkWidth, &bufLen, 0);
                    if (ret == CR_SUCCESS) {
                        FF_DEBUG("PCIe max link width: %u", entry->maxLinkWidth);
                    } else {
                        FF_DEBUG("Failed to get PCIe max link width: %s", ffDebugConfigRet(ret));
                    }
                }

                bufLen = sizeof(entry->currentLinkSpeed);
                ret = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_PciDevice_CurrentLinkSpeed, &propType, (PBYTE) &entry->currentLinkSpeed, &bufLen, 0);
                if (ret == CR_SUCCESS) {
                    FF_DEBUG("PCIe GEN: %u", entry->currentLinkSpeed);
                } else {
                    FF_DEBUG("Failed to get PCIe GEN: %s", ffDebugConfigRet(ret));
                }

                if (entry->currentLinkSpeed != FF_GPU_PCIE_SPEED_UNSET) {
                    bufLen = sizeof(entry->currentLinkWidth);
                    ret = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_PciDevice_CurrentLinkWidth, &propType, (PBYTE) &entry->currentLinkWidth, &bufLen, 0);
                    if (ret == CR_SUCCESS) {
                        FF_DEBUG("PCIe current link width: %u", entry->currentLinkWidth);
                    } else {
                        FF_DEBUG("Failed to get PCIe current link width: %s", ffDebugConfigRet(ret));
                    }
                }
            }
        }
    }

    FF_LIST_FOR_EACH (CacheEntry, entry, deviceIdsCache) {
        if (luid == entry->luid || entry->pciAddr == gpu->deviceId) {
            FF_DEBUG("Cache hit for adapter LUID: %08llX", luid);
            if (outDeviceIds->VendorID == -1u) {
                *outDeviceIds = entry->deviceIds;
            }

            if (gpu->vendor.length == 0) {
                if (entry->deviceIds.VendorID != -1u) {
                    ffStrbufSetStatic(&gpu->vendor, ffGPUGetVendorString(entry->deviceIds.VendorID));
                }
                if (gpu->vendor.length == 0 && entry->vendor.length > 0) {
                    ffStrbufDestroy(&gpu->vendor);
                    ffStrbufInitMove(&gpu->vendor, &entry->vendor);
                }
            } else if (gpu->name.length == 0 && entry->deviceIds.VendorID == -1u) {
                // Some Indirect display adapters reports fake Device IDs
                ffStrbufDestroy(&gpu->vendor);
                ffStrbufInitMove(&gpu->vendor, &entry->vendor);
            }

            if (gpu->name.length == 0) {
                ffStrbufDestroy(&gpu->name);
                ffStrbufInitMove(&gpu->name, &entry->name);
            }

            if (queryPcieGen) {
                gpu->psMax.gen = (uint16_t) entry->maxLinkSpeed;
                gpu->psMax.lanes = (uint16_t) entry->maxLinkWidth;
                gpu->psCurr.gen = (uint16_t) entry->currentLinkSpeed;
                gpu->psCurr.lanes = (uint16_t) entry->currentLinkWidth;
            }
            return true;
        }
    }

    FF_DEBUG("Cache miss for adapter LUID: %08llX", luid);
    return false;
}

#else
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/ioctl.h>
    #include <uchar.h>

int dxgfd = -2;

static void ffStrbufSetWS(FFstrbuf* strbuf, const char16_t* str) {
    ffStrbufClear(strbuf);

    mbstate_t state = {};
    while (*str) {
        char buf[5];
        size_t len = c16rtomb(buf, *str, &state);
        if (len == (size_t) -1) {
            ffStrbufAppendS(strbuf, "�"); // U+FFFD REPLACEMENT CHARACTER
        } else if (len > 0) {
            ffStrbufAppendNS(strbuf, (uint32_t) len, buf);
        }
        str++;
    }
}

static void closeDxgfd(void) {
    if (dxgfd >= 0) {
        close(dxgfd);
        dxgfd = -2;
        FF_DEBUG("Closed /dev/dxg file descriptor");
    }
}

[[maybe_unused]] static inline const char* ffDebugNtStatus(NTSTATUS status) {
    return status < 0 ? strerror(-status) : "Success";
}
#endif

const char*
#if _WIN32
ffDetectGPUImpl
#else
ffGPUDetectWsl2
#endif
    (const FFGPUOptions* options, FFlist* gpus) {
#if __linux__
    if (dxgfd == -2) {
        dxgfd = open("/dev/dxg", O_RDWR); // Windows DXCore/D3DKMT adapter driver for WSL
        if (dxgfd < 0) {
            if (errno == ENOENT) {
                FF_DEBUG("/dev/dxg is not available, WSL DXCore GPU driver not detected");
                return "No DXCore GPU driver detected (no /dev/dxg)";
            } else {
                FF_DEBUG("Failed to open /dev/dxg: %s", strerror(errno));
                return "Failed to open /dev/dxg";
            }
        }
        FF_DEBUG("Opened /dev/dxg successfully");
        atexit(closeDxgfd);
    }
    if (dxgfd < 0) {
        return "Failed to open /dev/dxg";
    }
#endif

#if FF_WIN81_COMPAT
    D3DKMT_ENUMADAPTERS enumAdapters = {};
    D3DKMT_ADAPTERINFO* const adapters = enumAdapters.Adapters;
    NTSTATUS status = D3DKMTEnumAdapters(&enumAdapters);
#else
    D3DKMT_ADAPTERINFO adapters[64];
    D3DKMT_ENUMADAPTERS2 enumAdapters = {
        .NumAdapters = ARRAY_SIZE(adapters),
        .pAdapters = adapters,
    };
    NTSTATUS status = D3DKMTEnumAdapters2(&enumAdapters);
#endif
    if (!NT_SUCCESS(status)) {
        FF_DEBUG("D3DKMTEnumAdapters(2) failed: %s", ffDebugNtStatus(status));
        return "Failed to enumerate adapters with D3DKMTEnumAdapters2";
    }

    FF_DEBUG("D3DKMTEnumAdapters(2) succeeded, adapter count: %" PRIu32, (uint32_t) enumAdapters.NumAdapters);

    for (uint32_t i = 0; i < enumAdapters.NumAdapters; i++) {
        const D3DKMT_ADAPTERINFO* adapter = &adapters[i];
        FF_DEBUG("Processing adapter #%u", i);

        D3DKMT_ADAPTERTYPE adapterType;
        status = D3DKMTQueryAdapterInfo(&(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_ADAPTERTYPE,
            .pPrivateDriverData = &adapterType,
            .PrivateDriverDataSize = sizeof(adapterType),
        });
        if (!NT_SUCCESS(status)) {
            FF_DEBUG("KMTQAITYPE_ADAPTERTYPE query failed for adapter #%u: %s", i, ffDebugNtStatus(status));
            goto close_adapter;
        }
        if (adapterType.SoftwareDevice) {
            FF_DEBUG("Skipping software adapter #%u", i);
            goto close_adapter;
        }

        uint64_t luid = ((uint64_t) adapter->AdapterLuid.HighPart << 32) | (uint64_t) adapter->AdapterLuid.LowPart;

        FFGPUResult* gpu = FF_LIST_ADD(FFGPUResult, *gpus);
        ffStrbufInit(&gpu->vendor);
        ffStrbufInit(&gpu->name);
        ffStrbufInit(&gpu->driver);
        ffStrbufInit(&gpu->platformApi);
        ffStrbufInit(&gpu->memoryType);
        gpu->index = FF_GPU_INDEX_UNSET;
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = 0;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;
        gpu->type = adapterType.HybridIntegrated
            ? FF_GPU_TYPE_INTEGRATED
            : adapterType.HybridDiscrete
            ? FF_GPU_TYPE_DISCRETE
            : FF_GPU_TYPE_UNKNOWN;
        gpu->pcieSpeed = FF_GPU_PCIE_SPEED_UNSET;

        D3DKMT_DRIVERVERSION wddmVersion;
        status = D3DKMTQueryAdapterInfo(&(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_DRIVERVERSION,
            .pPrivateDriverData = &wddmVersion,
            .PrivateDriverDataSize = sizeof(wddmVersion),
        });
        if (NT_SUCCESS(status)) {
            ffStrbufSetF(&gpu->platformApi, "WDDM %u.%u", (uint32_t) wddmVersion / 1000, ((uint32_t) wddmVersion % 1000) / 100);
            FF_DEBUG("Adapter #%u WDDM version: %u", i, (uint32_t) wddmVersion);
        } else {
            wddmVersion = KMT_DRIVERVERSION_WDDM_1_3; // Smallest supported WDDM version, used for fallback
            ffStrbufSetStatic(&gpu->platformApi, "WDDM");
            FF_DEBUG("KMTQAITYPE_DRIVERVERSION query failed for adapter #%u", i);
        }

        D3DKMT_ADAPTERADDRESS adapterAddress = {};
        status = D3DKMTQueryAdapterInfo(&(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_ADAPTERADDRESS,
            .pPrivateDriverData = &adapterAddress,
            .PrivateDriverDataSize = sizeof(adapterAddress),
        });
        if (NT_SUCCESS(status) && adapterAddress.FunctionNumber != 0xFFFF /* non-PCI device */) {
            gpu->deviceId = ffGPUPciAddr2Id(0, adapterAddress.BusNumber, adapterAddress.DeviceNumber, adapterAddress.FunctionNumber);
            FF_DEBUG("Adapter #%u PCI address: bus=%u device=%u function=%u",
                i,
                adapterAddress.BusNumber,
                adapterAddress.DeviceNumber,
                adapterAddress.FunctionNumber);
        } else {
            adapterAddress.BusNumber = -1u;
            gpu->deviceId = ffGPUGeneral2Id(luid);
            FF_DEBUG("KMTQAITYPE_ADAPTERADDRESS query failed for adapter #%u, fallback to LUID-based deviceId: %s",
                i,
                ffDebugNtStatus(status));
        }

        D3DKMT_QUERY_DEVICE_IDS deviceIds = { .PhysicalAdapterIndex = 0 };
        status = D3DKMTQueryAdapterInfo(&(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_PHYSICALADAPTERDEVICEIDS,
            .pPrivateDriverData = &deviceIds,
            .PrivateDriverDataSize = sizeof(deviceIds),
        });
        if (NT_SUCCESS(status)) {
            ffStrbufSetStatic(&gpu->vendor, ffGPUGetVendorString(deviceIds.DeviceIds.VendorID));
            FF_DEBUG("Adapter #%u vendor/device IDs: vendor=0x%04x device=0x%04x",
                i,
                deviceIds.DeviceIds.VendorID,
                deviceIds.DeviceIds.DeviceID);
        } else {
            deviceIds.DeviceIds.VendorID = -1u;
            FF_DEBUG("KMTQAITYPE_PHYSICALADAPTERDEVICEIDS query failed for adapter #%u: %s", i, ffDebugNtStatus(status));
        }

        D3DKMT_UMD_DRIVER_VERSION umdDriverVersion;
        status = D3DKMTQueryAdapterInfo(&(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_UMD_DRIVER_VERSION,
            .pPrivateDriverData = &umdDriverVersion,
            .PrivateDriverDataSize = sizeof(umdDriverVersion),
        });
        if (NT_SUCCESS(status)) {
            ffStrbufSetF(&gpu->driver,
                "%u.%u.%u.%u",
                (uint32_t) (umdDriverVersion.DriverVersion.QuadPart >> 48ul & 0xFFFF),
                (uint32_t) (umdDriverVersion.DriverVersion.QuadPart >> 32ul & 0xFFFF),
                (uint32_t) (umdDriverVersion.DriverVersion.QuadPart >> 16ul & 0xFFFF),
                (uint32_t) (umdDriverVersion.DriverVersion.QuadPart >> 0ul & 0xFFFF));
            FF_DEBUG("Adapter #%u UMD driver version: %08" PRIX64, i, (uint64_t) umdDriverVersion.DriverVersion.QuadPart);
        } else {
            FF_DEBUG("KMTQAITYPE_UMD_DRIVER_VERSION query failed for adapter #%u: %s", i, ffDebugNtStatus(status));
        }

        bool isIndirectDisplayDevice = gpu->type == FF_GPU_TYPE_UNKNOWN && adapterType.IndirectDisplayDevice;
        if (isIndirectDisplayDevice) {
            FF_DEBUG("Adapter #%u is an indirect display device", i);
        }

        if (!isIndirectDisplayDevice) {
            typeof(&ffDetectNvidiaGpuInfo) detectFn;
            const char* dllName;
            if (options->driverSpecific && getDriverSpecificDetectionFn(gpu->vendor.chars, &detectFn, &dllName)) {
                FF_DEBUG("Calling driver-specific detection function for vendor: %s, DLL: %s", gpu->vendor.chars, dllName);
                [[maybe_unused]] const char* error = detectFn(
                    &(FFGpuDriverCondition) {
                        .type = FF_GPU_DRIVER_CONDITION_TYPE_LUID |
                            (deviceIds.DeviceIds.VendorID != -1u ? FF_GPU_DRIVER_CONDITION_TYPE_DEVICE_ID : 0) |
                            (adapterAddress.BusNumber != -1u ? FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID : 0),
                        .pciDeviceId = {
                            .deviceId = deviceIds.DeviceIds.DeviceID,
                            .vendorId = deviceIds.DeviceIds.VendorID,
                            .subSystemId = deviceIds.DeviceIds.SubSystemID,
                            .revId = deviceIds.DeviceIds.RevisionID,
                        },
                        .pciBusId = {
                            .domain = 0,
                            .bus = adapterAddress.BusNumber,
                            .device = adapterAddress.DeviceNumber,
                            .func = adapterAddress.FunctionNumber,
                        },
                        .luid = luid,
                    },
                    (FFGpuDriverResult) {
                        .index = &gpu->index,
                        .temp = options->temp ? &gpu->temperature : nullptr,
                        .memory = options->driverSpecific ? &gpu->dedicated : nullptr,
                        .sharedMemory = options->driverSpecific ? &gpu->shared : nullptr,
                        .memoryType = options->driverSpecific ? &gpu->memoryType : nullptr,
                        .coreCount = options->driverSpecific ? (uint32_t*) &gpu->coreCount : nullptr,
                        .coreUsage = options->driverSpecific ? &gpu->coreUsage : nullptr,
                        .type = &gpu->type,
                        .frequency = options->driverSpecific ? &gpu->frequency : nullptr,
                        .name = &gpu->name,
                        .psCurr = options->driverSpecific ? &gpu->psCurr : nullptr,
                        .psMax = options->driverSpecific ? &gpu->psMax : nullptr,
                    },
                    dllName);
                FF_DEBUG("Driver-specific detection completed: %s", error ?: "Success");
            } else if (options->driverSpecific) {
                FF_DEBUG("No driver-specific detection function found for vendor: %s", gpu->vendor.chars);
            }
        };

        if (gpu->name.length == 0) {
            D3DKMT_ADAPTERREGISTRYINFO registryInfo;
            status = D3DKMTQueryAdapterInfo(&(D3DKMT_QUERYADAPTERINFO) {
                .hAdapter = adapter->hAdapter,
                .Type = KMTQAITYPE_ADAPTERREGISTRYINFO,
                .pPrivateDriverData = &registryInfo,
                .PrivateDriverDataSize = sizeof(registryInfo),
            });
            if (NT_SUCCESS(status)) {
                ffStrbufSetWS(&gpu->name, registryInfo.AdapterString);
                FF_DEBUG("Adapter #%u adapter string: %s", i, gpu->name.chars);
            } else {
                FF_DEBUG("KMTQAITYPE_ADAPTERREGISTRYINFO query failed for adapter #%u: %s", i, ffDebugNtStatus(status));
            }
        }

        #if _WIN32
        if (gpu->name.length == 0 || gpu->vendor.length == 0 || (options->driverSpecific && gpu->pcieSpeed == FF_GPU_PCIE_SPEED_UNSET)) {
            queryDeviceInfoCM(gpu, luid, &deviceIds.DeviceIds, options->driverSpecific);
        }
        #endif

        if (!isIndirectDisplayDevice) {
            if (gpu->dedicated.total == FF_GPU_VMEM_SIZE_UNSET && gpu->shared.total == FF_GPU_VMEM_SIZE_UNSET) {
                if (wddmVersion >= KMT_DRIVERVERSION_WDDM_3_1 && options->driverSpecific) {
                    // Supports memory usage query; requires Windows 11 (22H2) or later
                    D3DKMT_QUERYSTATISTICS queryStatistics = {
                        .Type = D3DKMT_QUERYSTATISTICS_SEGMENT_GROUP_USAGE,
                        .AdapterLuid = adapter->AdapterLuid,
                        .QuerySegmentGroupUsage = {
                            .PhysicalAdapterIndex = 0,
                            .SegmentGroup = D3DKMT_MEMORY_SEGMENT_GROUP_LOCAL,
                        },
                    };
                    status = D3DKMTQueryStatistics(&queryStatistics);
                    if (NT_SUCCESS(status)) {
                        D3DKMT_QUERYSTATISTICS_MEMORY_USAGE* info = &queryStatistics.QueryResult.SegmentGroupUsageInformation;
                        uint64_t used = info->AllocatedBytes + info->ModifiedBytes + info->StandbyBytes;
                        uint64_t total = used + info->FreeBytes + info->ZeroBytes;
                        gpu->dedicated.used = used;
                        gpu->dedicated.total = total;
                        FF_DEBUG("Adapter #%u local memory usage: used=%" PRIu64 " total=%" PRIu64, i, used, total);
                    } else {
                        FF_DEBUG("D3DKMT_QUERYSTATISTICS_SEGMENT_GROUP_USAGE (LOCAL) failed for adapter #%u: %s",
                            i,
                            ffDebugNtStatus(status));
                    }

                    queryStatistics.QuerySegmentGroupUsage.SegmentGroup = D3DKMT_MEMORY_SEGMENT_GROUP_NON_LOCAL;
                    status = D3DKMTQueryStatistics(&queryStatistics);
                    if (NT_SUCCESS(status)) {
                        D3DKMT_QUERYSTATISTICS_MEMORY_USAGE* info = &queryStatistics.QueryResult.SegmentGroupUsageInformation;
                        uint64_t used = info->AllocatedBytes + info->ModifiedBytes + info->StandbyBytes;
                        uint64_t total = used + info->FreeBytes + info->ZeroBytes;
                        gpu->shared.used = used;
                        gpu->shared.total = total;
                        FF_DEBUG("Adapter #%u non-local memory usage: used=%" PRIu64 " total=%" PRIu64, i, used, total);
                    } else {
                        FF_DEBUG("D3DKMT_QUERYSTATISTICS_SEGMENT_GROUP_USAGE (NON_LOCAL) failed for adapter #%u: %s",
                            i,
                            ffDebugNtStatus(status));
                    }
                } else {
                    // Supports basic segment (total) size query
                    D3DKMT_SEGMENTSIZEINFO segmentSizeInfo = {};
                    status = D3DKMTQueryAdapterInfo(&(D3DKMT_QUERYADAPTERINFO) {
                        .hAdapter = adapter->hAdapter,
                        .Type = KMTQAITYPE_GETSEGMENTSIZE,
                        .pPrivateDriverData = &segmentSizeInfo,
                        .PrivateDriverDataSize = sizeof(segmentSizeInfo),
                    });
                    if (NT_SUCCESS(status)) {
                        FF_DEBUG("Adapter #%u segment size - DedicatedVideoMemorySize: %" PRIu64
                                ", DedicatedSystemMemorySize: %" PRIu64 ", SharedSystemMemorySize: %" PRIu64,
                            i,
                            (uint64_t) segmentSizeInfo.DedicatedVideoMemorySize,
                            (uint64_t) segmentSizeInfo.DedicatedSystemMemorySize,
                            (uint64_t) segmentSizeInfo.SharedSystemMemorySize);
                        gpu->dedicated.total = segmentSizeInfo.DedicatedVideoMemorySize;
                        gpu->shared.total = segmentSizeInfo.DedicatedSystemMemorySize + segmentSizeInfo.SharedSystemMemorySize;
                    } else {
                        FF_DEBUG("Failed to query segment size information for adapter #%u: %s", i, ffDebugNtStatus(status));
                    }
                }
            }

            if (wddmVersion >= KMT_DRIVERVERSION_WDDM_2_4) {
                if (gpu->frequency == FF_GPU_FREQUENCY_UNSET) {
                    for (uint32_t nodeIdx = 0;; nodeIdx++) {
                        D3DKMT_NODEMETADATA nodeMetadata = {
                            .NodeOrdinalAndAdapterIndex = (0 << 16) | nodeIdx,
                        };
                        status = D3DKMTQueryAdapterInfo(&(D3DKMT_QUERYADAPTERINFO) {
                            .hAdapter = adapter->hAdapter,
                            .Type = KMTQAITYPE_NODEMETADATA,
                            .pPrivateDriverData = &nodeMetadata,
                            .PrivateDriverDataSize = sizeof(nodeMetadata),
                        });
                        if (!NT_SUCCESS(status)) { break; }

                        if (nodeMetadata.NodeData.EngineType != DXGK_ENGINE_TYPE_3D) { continue; }

                        D3DKMT_NODE_PERFDATA nodePerfData = {
                            .NodeOrdinal = nodeIdx,
                            .PhysicalAdapterIndex = 0,
                        };
                        status = D3DKMTQueryAdapterInfo(&(D3DKMT_QUERYADAPTERINFO) {
                            .hAdapter = adapter->hAdapter,
                            .Type = KMTQAITYPE_NODEPERFDATA,
                            .pPrivateDriverData = &nodePerfData,
                            .PrivateDriverDataSize = sizeof(nodePerfData),
                        });
                        if (NT_SUCCESS(status)) {
                            if (nodePerfData.MaxFrequency != 0) {
                                gpu->frequency = (uint32_t) (nodePerfData.MaxFrequency / 1000 / 1000);
                                FF_DEBUG("Adapter #%u max graphics frequency: %u MHz", i, gpu->frequency);
                            } else {
                                FF_DEBUG("Adapter #%u does not report max graphics frequency", i);
                            }
                            break;
                        } else {
                            FF_DEBUG("Failed to query node performance data for adapter #%u node #%u: %s",
                                i,
                                nodeIdx,
                                ffDebugNtStatus(status));
                        }
                    }
                }

                if (options->temp && gpu->temperature == FF_GPU_TEMP_UNSET) {
                    D3DKMT_ADAPTER_PERFDATA adapterPerfData = {
                        .PhysicalAdapterIndex = 0,
                    };
                    status = D3DKMTQueryAdapterInfo(&(D3DKMT_QUERYADAPTERINFO) {
                        .hAdapter = adapter->hAdapter,
                        .Type = KMTQAITYPE_ADAPTERPERFDATA,
                        .pPrivateDriverData = &adapterPerfData,
                        .PrivateDriverDataSize = sizeof(adapterPerfData),
                    });
                    if (NT_SUCCESS(status)) {
                        if (adapterPerfData.Temperature != 0) {
                            gpu->temperature = adapterPerfData.Temperature / 10.0;
                            FF_DEBUG("Adapter #%u temperature: %.1f°C", i, gpu->temperature);
                        } else {
                            FF_DEBUG("Adapter #%u does not report temperature data", i);
                        }
                    } else {
                        FF_DEBUG("Failed to query temperature for adapter #%u: %s", i, ffDebugNtStatus(status));
                    }
                }
            }

            if (gpu->type == FF_GPU_TYPE_UNKNOWN) {
                if (ffGPUDetectTypeByVendorAndName(gpu)) {
                    // OK
                }
    #if _WIN32
                else if (ffIsWindows10OrGreater()) {
                    const char* ffGPUDetectTypeWithDXCore(LUID adapterLuid, FFGPUResult * gpu);
                    [[maybe_unused]] const char* error = ffGPUDetectTypeWithDXCore(adapter->AdapterLuid, gpu);
                    FF_DEBUG("DXCore GPU type detection result: %s", error ?: "Success");
                }
    #endif
                else {
                    FF_DEBUG("Unable to determine GPU type by any method for this adapter");
                }
            }
        }

        FF_DEBUG("Adapter #%u summary: name='%s', vendor='%s', type=%u, deviceId=%" PRIu64,
            i,
            gpu->name.length ? gpu->name.chars : "unknown",
            gpu->vendor.length ? gpu->vendor.chars : "unknown",
            (uint32_t) gpu->type,
            (uint64_t) gpu->deviceId);

    close_adapter:
        status = D3DKMTCloseAdapter(&(D3DKMT_CLOSEADAPTER) { .hAdapter = adapter->hAdapter });
        if (NT_SUCCESS(status)) {
            FF_DEBUG("Closed adapter #%u successfully", i);
        } else {
            FF_DEBUG("Failed to close adapter #%u: %s", i, ffDebugNtStatus(status));
        }
    }

    return nullptr;
}
