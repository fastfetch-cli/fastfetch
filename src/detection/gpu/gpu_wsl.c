#if __x86_64__ || __aarch64__ // WSL2 only supports x86_64 and aarch64

#include "detection/gpu/gpu.h"
#include "detection/gpu/gpu_driver_specific.h"
#include "common/io.h"
#include "common/debug.h"

#include "d3dkmthk.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <uchar.h>

static void ffStrbufAppendUtf16(FFstrbuf* strbuf, const char16_t* str)
{
    mbstate_t state = {};
    while (*str)
    {
        char buf[5];
        size_t len = c16rtomb(buf, *str, &state);
        if (len == (size_t) -1)
            ffStrbufAppendS(strbuf, "�"); // U+FFFD REPLACEMENT CHARACTER
        else if (len > 0)
            ffStrbufAppendNS(strbuf, (uint32_t) len, buf);
        str++;
    }
}

const char* ffGPUDetectWsl2(const FFGPUOptions* options, FFlist* gpus)
{
    FF_AUTO_CLOSE_FD int dxg = open("/dev/dxg", O_RDWR); // Windows DXCore/D3DKMT adapter driver for WSL
    if (dxg < 0)
    {
        if (errno == ENOENT)
        {
            FF_DEBUG("/dev/dxg is not available, WSL DXCore GPU driver not detected");
            return "No DXCore GPU driver detected (no /dev/dxg)";
        }
        else
        {
            FF_DEBUG("Failed to open /dev/dxg: %s", strerror(errno));
            return "Failed to open /dev/dxg";
        }
    }

    FF_DEBUG("Opened /dev/dxg successfully");

    D3DKMT_ADAPTERINFO adapters[64];
    D3DKMT_ENUMADAPTERS2 enumAdapters = {
        .NumAdapters = ARRAY_SIZE(adapters),
        .pAdapters = adapters,
    };
    if (!NT_SUCCESS(D3DKMTEnumAdapters2(dxg, &enumAdapters)))
    {
        FF_DEBUG("D3DKMTEnumAdapters2 failed: %s", strerror(errno));
        return "Failed to enumerate adapters with D3DKMTEnumAdapters2";
    }

    FF_DEBUG("D3DKMTEnumAdapters2 succeeded, adapter count: %u", enumAdapters.NumAdapters);

    for (uint32_t i = 0; i < enumAdapters.NumAdapters; i++)
    {
        const D3DKMT_ADAPTERINFO* adapter = &adapters[i];
        FF_DEBUG("Processing adapter #%u", i);

        D3DKMT_ADAPTERTYPE adapterType;
        if (!NT_SUCCESS(D3DKMTQueryAdapterInfo(dxg, &(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_ADAPTERTYPE,
            .pPrivateDriverData = &adapterType,
            .PrivateDriverDataSize = sizeof(adapterType),
        })))
        {
            FF_DEBUG("KMTQAITYPE_ADAPTERTYPE query failed for adapter #%u: %s", i, strerror(errno));
            continue;
        }
        if (adapterType.SoftwareDevice)
        {
            FF_DEBUG("Skipping software adapter #%u", i);
            continue;
        }

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

        const char* vendorStr = NULL;

        D3DKMT_QUERY_DEVICE_IDS deviceIds = { .PhysicalAdapterIndex = 0 };
        if (NT_SUCCESS(D3DKMTQueryAdapterInfo(dxg, &(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_PHYSICALADAPTERDEVICEIDS,
            .pPrivateDriverData = &deviceIds,
            .PrivateDriverDataSize = sizeof(deviceIds),
        })))
        {
            vendorStr = ffGPUGetVendorString(deviceIds.DeviceIds.VendorID);
            ffStrbufSetStatic(&gpu->vendor, vendorStr);
            FF_DEBUG("Adapter #%u vendor/device IDs: vendor=0x%04x device=0x%04x", i, deviceIds.DeviceIds.VendorID, deviceIds.DeviceIds.DeviceID);
        }
        else
            FF_DEBUG("KMTQAITYPE_PHYSICALADAPTERDEVICEIDS query failed for adapter #%u: %s", i, strerror(errno));

        D3DKMT_ADAPTERADDRESS adapterAddress;
        if (NT_SUCCESS(D3DKMTQueryAdapterInfo(dxg, &(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_ADAPTERADDRESS,
            .pPrivateDriverData = &adapterAddress,
            .PrivateDriverDataSize = sizeof(adapterAddress),
        })))
        {
            gpu->deviceId = ffGPUPciAddr2Id(0, adapterAddress.BusNumber, adapterAddress.DeviceNumber, adapterAddress.FunctionNumber);
            FF_DEBUG("Adapter #%u PCI address: bus=%u device=%u function=%u", i, adapterAddress.BusNumber, adapterAddress.DeviceNumber, adapterAddress.FunctionNumber);
        }
        else
        {
            adapterAddress.BusNumber = -1u;
            gpu->deviceId = ffGPUGeneral2Id(((uint64_t)adapter->AdapterLuid.HighPart << 32) | (uint64_t)adapter->AdapterLuid.LowPart);
            FF_DEBUG("KMTQAITYPE_ADAPTERADDRESS query failed for adapter #%u, fallback to LUID-based deviceId: %s", i, strerror(errno));
        }

        D3DKMT_UMD_DRIVER_VERSION umdDriverVersion;
        if (NT_SUCCESS(D3DKMTQueryAdapterInfo(dxg, &(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_UMD_DRIVER_VERSION,
            .pPrivateDriverData = &umdDriverVersion,
            .PrivateDriverDataSize = sizeof(umdDriverVersion),
        })))
        {
            ffStrbufSetF(&gpu->driver, "%lu.%lu.%lu.%lu",
                umdDriverVersion.DriverVersion.QuadPart >> 48ul & 0xFFFF,
                umdDriverVersion.DriverVersion.QuadPart >> 32ul & 0xFFFF,
                umdDriverVersion.DriverVersion.QuadPart >> 16ul & 0xFFFF,
                umdDriverVersion.DriverVersion.QuadPart >> 0ul  & 0xFFFF);
            FF_DEBUG("Adapter #%u UMD driver version: %08lX", i, (unsigned long) umdDriverVersion.DriverVersion.QuadPart);
        }
        else
        {
            FF_DEBUG("KMTQAITYPE_UMD_DRIVER_VERSION query failed for adapter #%u: %s", i, strerror(errno));
        }

        D3DKMT_DRIVERVERSION wddmVersion = KMT_DRIVERVERSION_WDDM_3_0;
        if (NT_SUCCESS(D3DKMTQueryAdapterInfo(dxg, &(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_DRIVERVERSION,
            .pPrivateDriverData = &wddmVersion,
            .PrivateDriverDataSize = sizeof(wddmVersion),
        })))
        {
            ffStrbufSetF(&gpu->platformApi, "WDDM %u.%u", wddmVersion / 1000, (wddmVersion % 1000) / 100);
            FF_DEBUG("Adapter #%u WDDM version: %u", i, wddmVersion);
        }
        else
        {
            ffStrbufSetStatic(&gpu->platformApi, "WDDM");
            FF_DEBUG("KMTQAITYPE_DRIVERVERSION query failed for adapter #%u", i);
        }

        if (gpu->name.length == 0)
        {
            D3DKMT_ADAPTERREGISTRYINFO registryInfo;
            if (NT_SUCCESS(D3DKMTQueryAdapterInfo(dxg, &(D3DKMT_QUERYADAPTERINFO) {
                .hAdapter = adapter->hAdapter,
                .Type = KMTQAITYPE_ADAPTERREGISTRYINFO,
                .pPrivateDriverData = &registryInfo,
                .PrivateDriverDataSize = sizeof(registryInfo),
            })))
            {
                ffStrbufAppendUtf16(&gpu->name, registryInfo.AdapterString);
                FF_DEBUG("Adapter #%u adapter string: %s", i, gpu->name.chars);
            }
            else
                FF_DEBUG("KMTQAITYPE_ADAPTERREGISTRYINFO query failed for adapter #%u: %s", i, strerror(errno));
        }

        if (vendorStr == FF_GPU_VENDOR_NAME_NVIDIA && options->driverSpecific)
        {
            FF_DEBUG("Trying NVIDIA driver-specific detection for adapter #%u", i);
            FFGpuDriverCondition cond = {
                .type = FF_GPU_DRIVER_CONDITION_TYPE_DEVICE_ID | (adapterAddress.DeviceNumber != -1u ? FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID : 0),
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
            };
            const char* error = ffDetectNvidiaGpuInfo(&cond, (FFGpuDriverResult){
                .index = &gpu->index,
                .temp = options->temp ? &gpu->temperature : NULL,
                .memory = options->driverSpecific ? &gpu->dedicated : NULL,
                .coreCount = options->driverSpecific ? (uint32_t*) &gpu->coreCount : NULL,
                .coreUsage = options->driverSpecific ? &gpu->coreUsage : NULL,
                .type = &gpu->type,
                .frequency = options->driverSpecific ? &gpu->frequency : NULL,
                .name = &gpu->name,
            }, "/usr/lib/wsl/lib/libnvidia-ml.so");
            if (error)
                FF_DEBUG("NVIDIA driver-specific detection failed for adapter #%u: %s", i, error);
            else
                FF_DEBUG("NVIDIA driver-specific detection succeeded for adapter #%u", i);
        }

        if (gpu->dedicated.total == FF_GPU_VMEM_SIZE_UNSET && gpu->shared.total == FF_GPU_VMEM_SIZE_UNSET)
        {
            if (wddmVersion >= KMT_DRIVERVERSION_WDDM_3_1 && options->driverSpecific)
            {
                // Supports memory usage query; requires Windows 11 (22H2) or later
                D3DKMT_QUERYSTATISTICS queryStatistics = {
                    .Type = D3DKMT_QUERYSTATISTICS_SEGMENT_GROUP_USAGE,
                    .AdapterLuid = adapter->AdapterLuid,
                    .QuerySegmentGroupUsage = {
                        .PhysicalAdapterIndex = 0,
                        .SegmentGroup = D3DKMT_MEMORY_SEGMENT_GROUP_LOCAL,
                    },
                };
                if (NT_SUCCESS(D3DKMTQueryStatistics(dxg, &queryStatistics)))
                {
                    D3DKMT_QUERYSTATISTICS_MEMORY_USAGE* info = &queryStatistics.QueryResult.SegmentGroupUsageInformation;
                    uint64_t used = info->AllocatedBytes + info->ModifiedBytes + info->StandbyBytes;
                    uint64_t total = used + info->FreeBytes + info->ZeroBytes;
                    gpu->dedicated.used = used;
                    gpu->dedicated.total = total;
                    FF_DEBUG("Adapter #%u local memory usage: used=%lu total=%lu", i, (unsigned long) used, (unsigned long) total);
                }
                else
                    FF_DEBUG("D3DKMT_QUERYSTATISTICS_SEGMENT_GROUP_USAGE (LOCAL) failed for adapter #%u: %s", i, strerror(errno));

                queryStatistics.QuerySegmentGroupUsage.SegmentGroup = D3DKMT_MEMORY_SEGMENT_GROUP_NON_LOCAL;
                if (NT_SUCCESS(D3DKMTQueryStatistics(dxg, &queryStatistics)))
                {
                    D3DKMT_QUERYSTATISTICS_MEMORY_USAGE* info = &queryStatistics.QueryResult.SegmentGroupUsageInformation;
                    uint64_t used = info->AllocatedBytes + info->ModifiedBytes + info->StandbyBytes;
                    uint64_t total = used + info->FreeBytes + info->ZeroBytes;
                    gpu->shared.used = used;
                    gpu->shared.total = total;
                    FF_DEBUG("Adapter #%u non-local memory usage: used=%lu total=%lu", i, (unsigned long) used, (unsigned long) total);
                }
                else
                    FF_DEBUG("D3DKMT_QUERYSTATISTICS_SEGMENT_GROUP_USAGE (NON_LOCAL) failed for adapter #%u: %s", i, strerror(errno));
            }
            else
            {
                // Supports basic segment (total) size query
                D3DKMT_SEGMENTSIZEINFO segmentSizeInfo = {};
                D3DKMT_QUERYADAPTERINFO queryAdapterInfo = {
                    .hAdapter = adapter->hAdapter,
                    .Type = KMTQAITYPE_GETSEGMENTSIZE,
                    .pPrivateDriverData = &segmentSizeInfo,
                    .PrivateDriverDataSize = sizeof(segmentSizeInfo),
                };
                if (NT_SUCCESS(D3DKMTQueryAdapterInfo(dxg, &queryAdapterInfo)))
                {
                    FF_DEBUG("Adapter #%u segment size - DedicatedVideoMemorySize: %lu, DedicatedSystemMemorySize: %lu, SharedSystemMemorySize: %lu",
                        i, segmentSizeInfo.DedicatedVideoMemorySize, segmentSizeInfo.DedicatedSystemMemorySize, segmentSizeInfo.SharedSystemMemorySize);
                    gpu->dedicated.total = segmentSizeInfo.DedicatedVideoMemorySize;
                    gpu->shared.total = segmentSizeInfo.DedicatedSystemMemorySize + segmentSizeInfo.SharedSystemMemorySize;
                }
                else
                {
                    FF_DEBUG("Failed to query segment size information for adapter #%u: %s", i, strerror(errno));
                }
            }
        }

        if (gpu->frequency == FF_GPU_FREQUENCY_UNSET && wddmVersion >= KMT_DRIVERVERSION_WDDM_3_1)
        {
            for (ULONG nodeIdx = 0; ; nodeIdx++)
            {
                D3DKMT_NODEMETADATA nodeMetadata = {
                    .NodeOrdinalAndAdapterIndex = (0 << 16) | nodeIdx,
                };
                if (!NT_SUCCESS(D3DKMTQueryAdapterInfo(dxg, &(D3DKMT_QUERYADAPTERINFO) {
                    .hAdapter = adapter->hAdapter,
                    .Type = KMTQAITYPE_NODEMETADATA,
                    .pPrivateDriverData = &nodeMetadata,
                    .PrivateDriverDataSize = sizeof(nodeMetadata),
                })))
                    break;

                if (nodeMetadata.NodeData.EngineType != DXGK_ENGINE_TYPE_3D)
                    continue;

                D3DKMT_QUERYSTATISTICS queryStatistics = {
                    .Type = D3DKMT_QUERYSTATISTICS_NODE2,
                    .AdapterLuid = adapter->AdapterLuid,
                    .QueryNode2 = { .PhysicalAdapterIndex = 0, .NodeOrdinal = (UINT16) nodeIdx },
                };
                if (NT_SUCCESS(D3DKMTQueryStatistics(dxg, &queryStatistics)))
                {
                    gpu->frequency = (uint32_t) (queryStatistics.QueryResult.NodeInformation.NodePerfData.MaxFrequency / 1000 / 1000);
                    FF_DEBUG("Adapter #%u max graphics frequency: %u MHz", i, gpu->frequency);
                    break;
                }
                else
                    FF_DEBUG("Failed to query node performance data for adapter #%u node #%u: %s", i, nodeIdx, strerror(errno));
            }
        }

        if (options->temp && gpu->temperature == FF_GPU_TEMP_UNSET)
        {
            D3DKMT_QUERYSTATISTICS queryStatistics = {
                .Type = D3DKMT_QUERYSTATISTICS_PHYSICAL_ADAPTER,
                .AdapterLuid = adapter->AdapterLuid,
                .QueryPhysAdapter = { .PhysicalAdapterIndex = 0 },
            };
            if (NT_SUCCESS(D3DKMTQueryStatistics(dxg, &queryStatistics)) &&
                queryStatistics.QueryResult.PhysAdapterInformation.AdapterPerfData.Temperature != 0)
            {
                gpu->temperature = queryStatistics.QueryResult.PhysAdapterInformation.AdapterPerfData.Temperature / 10.0;
                FF_DEBUG("Adapter #%u temperature: %.1f°C", i, gpu->temperature);
            }
            else
                FF_DEBUG("Failed to query temperature for adapter #%u: %s", i, strerror(errno));
        }

        if (gpu->type == FF_GPU_TYPE_UNKNOWN)
        {
            if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_NVIDIA)
            {
                if (ffStrbufStartsWithIgnCaseS(&gpu->name, "GeForce") ||
                    ffStrbufStartsWithIgnCaseS(&gpu->name, "Quadro") ||
                    ffStrbufStartsWithIgnCaseS(&gpu->name, "Tesla"))
                    gpu->type = FF_GPU_TYPE_DISCRETE;
            }
            else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_MTHREADS)
            {
                if (ffStrbufStartsWithIgnCaseS(&gpu->name, "MTT "))
                    gpu->type = FF_GPU_TYPE_DISCRETE;
            }
            else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_INTEL)
            {
                // 0000:00:02.0 is reserved for Intel integrated graphics
                gpu->type = gpu->deviceId == ffGPUPciAddr2Id(0, 0, 2, 0) ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;
            }
        }

        FF_DEBUG("Adapter #%u summary: name='%s', vendor='%s', type=%u, deviceId=%lu",
            i,
            gpu->name.length ? gpu->name.chars : "unknown",
            gpu->vendor.length ? gpu->vendor.chars : "unknown",
            (unsigned) gpu->type,
            (unsigned long) gpu->deviceId);

        if (NT_SUCCESS(D3DKMTCloseAdapter(dxg, &(D3DKMT_CLOSEADAPTER) {
            .hAdapter = adapter->hAdapter
        })))
            FF_DEBUG("Closed adapter #%u successfully", i);
        else
            FF_DEBUG("Failed to close adapter #%u: %s", i, strerror(errno));
    }

    return NULL;
}

#endif
