#if __x86_64__ || __aarch64__ // WSL2 only supports x86_64 and aarch64

#include "detection/gpu/gpu.h"
#include "detection/gpu/gpu_driver_specific.h"
#include "common/io.h"

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
            return "No DXCore GPU driver detected (no /dev/dxg)";
        else
            return "Failed to open /dev/dxg";
    }

    D3DKMT_ADAPTERINFO adapters[MAX_ENUM_ADAPTERS];
    D3DKMT_ENUMADAPTERS2 enumAdapters = {
        .NumAdapters = ARRAY_SIZE(adapters),
        .pAdapters = adapters,
    };
    if (!NT_SUCCESS(D3DKMTEnumAdapters2(dxg, &enumAdapters)))
        return "Failed to enumerate adapters with D3DKMTEnumAdapters2";

    for (uint32_t i = 0; i < enumAdapters.NumAdapters; i++)
    {
        const D3DKMT_ADAPTERINFO* adapter = &adapters[i];

        D3DKMT_ADAPTERTYPE adapterType;
        if (!NT_SUCCESS(D3DKMTQueryAdapterInfo(dxg, &(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_ADAPTERTYPE,
            .pPrivateDriverData = &adapterType,
            .PrivateDriverDataSize = sizeof(adapterType),
        }))) continue;
        if (adapterType.SoftwareDevice) continue;

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
        }

        D3DKMT_ADAPTERADDRESS adapterAddress = { .BusNumber = -1u };
        if (NT_SUCCESS(D3DKMTQueryAdapterInfo(dxg, &(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_ADAPTERADDRESS,
            .pPrivateDriverData = &adapterAddress,
            .PrivateDriverDataSize = sizeof(adapterAddress),
        })))
            gpu->deviceId = ffGPUPciAddr2Id(0, adapterAddress.BusNumber, adapterAddress.DeviceNumber, adapterAddress.FunctionNumber);
        else
            gpu->deviceId = ffGPUGeneral2Id(((uint64_t)adapter->AdapterLuid.HighPart << 32) | (uint64_t)adapter->AdapterLuid.LowPart);

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
        }

        D3DKMT_DRIVERVERSION kmdDriverVersion;
        if (NT_SUCCESS(D3DKMTQueryAdapterInfo(dxg, &(D3DKMT_QUERYADAPTERINFO) {
            .hAdapter = adapter->hAdapter,
            .Type = KMTQAITYPE_DRIVERVERSION,
            .pPrivateDriverData = &kmdDriverVersion,
            .PrivateDriverDataSize = sizeof(kmdDriverVersion),
        })))
            ffStrbufSetF(&gpu->platformApi, "WDDM %u.%u", kmdDriverVersion / 1000, (kmdDriverVersion % 1000) / 100);
        else
            ffStrbufSetStatic(&gpu->platformApi, "WDDM");

        if (gpu->name.length == 0)
        {
            D3DKMT_ADAPTERREGISTRYINFO registryInfo;
            if (NT_SUCCESS(D3DKMTQueryAdapterInfo(dxg, &(D3DKMT_QUERYADAPTERINFO) {
                .hAdapter = adapter->hAdapter,
                .Type = KMTQAITYPE_ADAPTERREGISTRYINFO,
                .pPrivateDriverData = &registryInfo,
                .PrivateDriverDataSize = sizeof(registryInfo),
            })))
                ffStrbufAppendUtf16(&gpu->name, registryInfo.AdapterString);
        }

        if (vendorStr == FF_GPU_VENDOR_NAME_NVIDIA && options->driverSpecific)
        {
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
            ffDetectNvidiaGpuInfo(&cond, (FFGpuDriverResult){
                .index = &gpu->index,
                .temp = options->temp ? &gpu->temperature : NULL,
                .memory = options->driverSpecific ? &gpu->dedicated : NULL,
                .coreCount = options->driverSpecific ? (uint32_t*) &gpu->coreCount : NULL,
                .coreUsage = options->driverSpecific ? &gpu->coreUsage : NULL,
                .type = &gpu->type,
                .frequency = options->driverSpecific ? &gpu->frequency : NULL,
                .name = &gpu->name,
            }, "/usr/lib/wsl/lib/libnvidia-ml.so");
        }

        if (gpu->dedicated.total == FF_GPU_VMEM_SIZE_UNSET && gpu->shared.total == FF_GPU_VMEM_SIZE_UNSET)
        {
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
            }

            queryStatistics.QuerySegmentGroupUsage.SegmentGroup = D3DKMT_MEMORY_SEGMENT_GROUP_NON_LOCAL;
            if (NT_SUCCESS(D3DKMTQueryStatistics(dxg, &queryStatistics)))
            {
                D3DKMT_QUERYSTATISTICS_MEMORY_USAGE* info = &queryStatistics.QueryResult.SegmentGroupUsageInformation;
                uint64_t used = info->AllocatedBytes + info->ModifiedBytes + info->StandbyBytes;
                uint64_t total = used + info->FreeBytes + info->ZeroBytes;
                gpu->shared.used = used;
                gpu->shared.total = total;
            }
        }

        if (gpu->frequency == FF_GPU_FREQUENCY_UNSET)
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
                    break;
                }
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
                gpu->temperature = queryStatistics.QueryResult.PhysAdapterInformation.AdapterPerfData.Temperature / 10.0;
        }

        (void) D3DKMTCloseAdapter(dxg, &(D3DKMT_CLOSEADAPTER) {
            .hAdapter = adapter->hAdapter
        });
    }

    return NULL;
}

#endif
