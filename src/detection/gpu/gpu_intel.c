#include "gpu_driver_specific.h"

#include "common/library.h"
#include "util/mallocHelper.h"
#include "igcl.h"

struct FFIgclData {
    FF_LIBRARY_SYMBOL(ctlClose)

    FF_LIBRARY_SYMBOL(ctlEnumerateDevices)
    FF_LIBRARY_SYMBOL(ctlGetDeviceProperties)
    FF_LIBRARY_SYMBOL(ctlEnumTemperatureSensors)
    FF_LIBRARY_SYMBOL(ctlTemperatureGetProperties)
    FF_LIBRARY_SYMBOL(ctlEnumMemoryModules)
    FF_LIBRARY_SYMBOL(ctlMemoryGetProperties)
    FF_LIBRARY_SYMBOL(ctlMemoryGetState)
    FF_LIBRARY_SYMBOL(ctlEnumFrequencyDomains)
    FF_LIBRARY_SYMBOL(ctlFrequencyGetProperties)

    bool inited;
    ctl_api_handle_t apiHandle;
} igclData;

static void shutdownIgcl()
{
    if (igclData.apiHandle)
    {
        igclData.ffctlClose(igclData.apiHandle);
        igclData.apiHandle = NULL;
    }
}

const char* ffDetectIntelGpuInfo(const FFGpuDriverCondition* cond, FFGpuDriverResult result, const char* soName)
{
    if (!igclData.inited)
    {
        igclData.inited = true;
        FF_LIBRARY_LOAD(libigcl, "dlopen igcl (ControlLib) failed", soName , 1);
        FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libigcl, ctlInit)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlClose)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlEnumerateDevices)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlGetDeviceProperties)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlEnumTemperatureSensors)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlTemperatureGetProperties)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlEnumMemoryModules)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlMemoryGetProperties)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlMemoryGetState)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlEnumFrequencyDomains)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlFrequencyGetProperties)

        if (ffctlInit(&(ctl_init_args_t) {
            .AppVersion = CTL_IMPL_VERSION,
            .flags = CTL_INIT_FLAG_USE_LEVEL_ZERO,
            .Size = sizeof(ctl_init_args_t),
            .Version = 0,
        }, &igclData.apiHandle) != CTL_RESULT_SUCCESS)
            return "loading igcl library failed";
        atexit(shutdownIgcl);
        libigcl = NULL; // don't close igcl
    }

    if (!igclData.apiHandle)
        return "loading igcl library failed";

    uint32_t deviceCount = 0;
    if (igclData.ffctlEnumerateDevices(igclData.apiHandle, &deviceCount, NULL))
        return "ctlEnumerateDevices(NULL) failed";
    if (deviceCount == 0)
        return "No Intel graphics adapter found";

    FF_AUTO_FREE ctl_device_adapter_handle_t* devices = malloc(deviceCount * sizeof(*devices));
    if (igclData.ffctlEnumerateDevices(igclData.apiHandle, &deviceCount, devices))
        return "ctlEnumerateDevices(devices) failed";

    ctl_device_adapter_handle_t device = NULL;

    uint64_t /* LUID */ deviceId = 0;
    ctl_device_adapter_properties_t properties = {
        .Size = sizeof(properties),
        .pDeviceID = &deviceId,
        .device_id_size = sizeof(deviceId),
        .Version = 2,
    };
    for (uint32_t iDev = 0; iDev < deviceCount; iDev++)
    {
        if (igclData.ffctlGetDeviceProperties(devices[iDev], &properties) != CTL_RESULT_SUCCESS)
            continue;

        if (properties.device_type != CTL_DEVICE_TYPE_GRAPHICS)
            continue;

        if (cond->type & FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID)
        {
            if (cond->pciBusId.bus == properties.adapter_bdf.bus &&
                cond->pciBusId.device == properties.adapter_bdf.device &&
                cond->pciBusId.func == properties.adapter_bdf.function)
            {
                device = devices[iDev];
                break;
            }
        }
        else if (cond->type & FF_GPU_DRIVER_CONDITION_TYPE_LUID)
        {
            if (cond->luid == deviceId)
            {
                device = devices[iDev];
                break;
            }
        }
        else if (cond->type & FF_GPU_DRIVER_CONDITION_TYPE_DEVICE_ID)
        {
            if (
                cond->pciDeviceId.deviceId == properties.pci_device_id &&
                cond->pciDeviceId.vendorId == properties.pci_vendor_id &&
                cond->pciDeviceId.subSystemId == (uint32_t) ((properties.pci_subsys_id << 16u) | properties.pci_subsys_vendor_id) &&
                cond->pciDeviceId.revId == properties.rev_id)
            {
                device = devices[iDev];
                break;
            }
        }
    }

    if (!device)
        return "Device not found";

    if (result.coreCount)
        *result.coreCount = properties.num_slices * properties.num_sub_slices_per_slice * properties.num_eus_per_sub_slice;

    if (result.memory)
    {
        ctl_mem_handle_t memoryModules[16];
        uint32_t memoryCount = ARRAY_SIZE(memoryModules);
        if (igclData.ffctlEnumMemoryModules(device, &memoryCount, memoryModules) == CTL_RESULT_SUCCESS && memoryCount > 0)
        {
            result.memory->used = 0;
            result.memory->total = 0;
            for (uint32_t iMem = 0; iMem < memoryCount; iMem++)
            {
                ctl_mem_properties_t memoryProperties = {
                    .Size = sizeof(memoryProperties),
                    .Version = 0,
                };
                if (igclData.ffctlMemoryGetProperties(memoryModules[iMem], &memoryProperties) == CTL_RESULT_SUCCESS)
                {
                    if (memoryProperties.location == CTL_MEM_LOC_DEVICE && result.memoryType)
                    {
                        switch (memoryProperties.type)
                        {
                            #define FF_ICTL_MEM_TYPE_CASE(type) case CTL_MEM_TYPE_##type: ffStrbufSetStatic(result.memoryType, #type); break
                            FF_ICTL_MEM_TYPE_CASE(HBM);
                            FF_ICTL_MEM_TYPE_CASE(DDR);
                            FF_ICTL_MEM_TYPE_CASE(DDR3);
                            FF_ICTL_MEM_TYPE_CASE(DDR4);
                            FF_ICTL_MEM_TYPE_CASE(DDR5);
                            FF_ICTL_MEM_TYPE_CASE(LPDDR);
                            FF_ICTL_MEM_TYPE_CASE(LPDDR3);
                            FF_ICTL_MEM_TYPE_CASE(LPDDR4);
                            FF_ICTL_MEM_TYPE_CASE(LPDDR5);
                            FF_ICTL_MEM_TYPE_CASE(GDDR4);
                            FF_ICTL_MEM_TYPE_CASE(GDDR5);
                            FF_ICTL_MEM_TYPE_CASE(GDDR5X);
                            FF_ICTL_MEM_TYPE_CASE(GDDR6);
                            FF_ICTL_MEM_TYPE_CASE(GDDR6X);
                            FF_ICTL_MEM_TYPE_CASE(GDDR7);
                            #undef FF_ICTL_MEM_TYPE_CASE
                            default:
                                ffStrbufSetF(result.memoryType, "Unknown (%u)", memoryProperties.type);
                                break;
                        }
                    }

                    ctl_mem_state_t memoryState = {
                        .Size = sizeof(ctl_mem_state_t),
                        .Version = 0,
                    };
                    if (igclData.ffctlMemoryGetState(memoryModules[iMem], &memoryState) == CTL_RESULT_SUCCESS)
                    {
                        if (memoryProperties.location == CTL_MEM_LOC_DEVICE)
                        {
                            result.memory->total += memoryState.size;
                            result.memory->used += memoryState.size - memoryState.free;
                        }
                        else if (result.sharedMemory && memoryProperties.location == CTL_MEM_LOC_SYSTEM)
                        {
                            result.sharedMemory->total += memoryState.size;
                            result.sharedMemory->used += memoryState.size - memoryState.free;
                        }
                    }
                }
            }
        }
    }

    if (result.type)
    {
        *result.type = properties.graphics_adapter_properties & CTL_ADAPTER_PROPERTIES_FLAG_INTEGRATED
            ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;
    }

    if (result.temp)
    {
        ctl_temp_handle_t sensors[16];
        uint32_t sensorCount = ARRAY_SIZE(sensors);
        if (igclData.ffctlEnumTemperatureSensors(device, &sensorCount, sensors) == CTL_RESULT_SUCCESS && sensorCount > 0)
        {
            for (uint32_t iSensor = 0; iSensor < sensorCount; iSensor++)
            {
                ctl_temp_properties_t props = { .Size = sizeof(props) };
                // The official sample code does not set Version
                // https://github.com/intel/drivers.gpu.control-library/blob/1bbacbf3814f2fd0d2b930cdf42fad83f3628db9/Samples/Telemetry_Samples/Sample_TelemetryAPP.cpp#L256
                if (igclData.ffctlTemperatureGetProperties(sensors[iSensor], &props) == CTL_RESULT_SUCCESS)
                {
                    if (props.type == CTL_TEMP_SENSORS_GPU)
                    {
                        *result.temp = props.maxTemperature;
                        break;
                    }
                }
            }
        }
    }

    if (result.frequency)
    {
        ctl_freq_handle_t domains[16];
        uint32_t domainCount = ARRAY_SIZE(domains);
        if (igclData.ffctlEnumFrequencyDomains(device, &domainCount, domains) == CTL_RESULT_SUCCESS && domainCount > 0)
        {
            double maxValue = 0;
            ctl_freq_properties_t props = { .Size = sizeof(props), .Version = 0 };
            for (uint32_t iDomain = 0; iDomain < domainCount; iDomain++)
            {
                if (igclData.ffctlFrequencyGetProperties(domains[iDomain], &props) == CTL_RESULT_SUCCESS)
                {
                    if (props.type == CTL_FREQ_DOMAIN_GPU && props.max > maxValue)
                        maxValue = props.max;
                }
            }
            *result.frequency = (uint32_t) (maxValue + 0.5);
        }
    }

    if (result.name)
        ffStrbufSetS(result.name, properties.name);

    return NULL;
}
