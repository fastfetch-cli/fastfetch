#include "gpu_intel.h"

#include "3rdparty/igcl/igcl_api.h"
#include "common/library.h"

struct FFIgclData {
    FF_LIBRARY_SYMBOL(ctlClose)

    FF_LIBRARY_SYMBOL(ctlEnumerateDevices)
    FF_LIBRARY_SYMBOL(ctlGetDeviceProperties)
    FF_LIBRARY_SYMBOL(ctlEnumTemperatureSensors)
    FF_LIBRARY_SYMBOL(ctlTemperatureGetState)
    FF_LIBRARY_SYMBOL(ctlEnumMemoryModules)
    FF_LIBRARY_SYMBOL(ctlMemoryGetState)

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

const char* ffDetectIntelGpuInfo(FFGpuIntelCondition cond, FFGpuIntelResult result, const char* soName)
{
    if (!igclData.inited)
    {
        igclData.inited = true;
        FF_LIBRARY_LOAD(libigcl, NULL, "dlopen igcl (ControlLib) failed", soName , 1);
        FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libigcl, ctlInit)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlClose)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlEnumerateDevices)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlGetDeviceProperties)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlEnumTemperatureSensors)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlTemperatureGetState)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlEnumMemoryModules)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libigcl, igclData, ctlMemoryGetState)

        if (ffctlInit(&(ctl_init_args_t) {
            .AppVersion = CTL_MAKE_VERSION(CTL_IMPL_MAJOR_VERSION, CTL_IMPL_MINOR_VERSION),
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
    ctl_device_adapter_handle_t devices[16] = {};
    if (deviceCount == 0 || deviceCount > sizeof(devices) / sizeof(devices[0]))
        return "Invalid device count";
    if (igclData.ffctlEnumerateDevices(igclData.apiHandle, &deviceCount, devices))
        return "ctlEnumerateDevices(devices) failed";

    ctl_device_adapter_handle_t device = NULL;

    LUID deviceId;
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

        if (
            cond.pciDeviceId == properties.pci_device_id &&
            cond.pciVendorId == properties.pci_vendor_id &&
            cond.pciSubSystemId == (((uint32_t) properties.pci_subsys_id << 16u) + (uint32_t) properties.pci_subsys_vendor_id) &&
            cond.revId == properties.rev_id)
        {
            device = devices[iDev];
            break;
        }
    }

    if (!device)
        return "Device not found";

    if (result.coreCount)
        *result.coreCount = properties.num_slices * properties.num_sub_slices_per_slice * properties.num_eus_per_sub_slice;

    if (result.memory)
    {
        ctl_mem_handle_t memoryModules[16];
        uint32_t memoryCount = sizeof(memoryModules) / sizeof(memoryModules[0]);
        if (igclData.ffctlEnumMemoryModules(device, &memoryCount, memoryModules) == CTL_RESULT_SUCCESS)
        {
            result.memory->used = 0;
            result.memory->total = 0;
            for (uint32_t iMem = 0; iMem < memoryCount; iMem++)
            {
                ctl_mem_state_t memoryState = {
                    .Size = sizeof(ctl_mem_state_t),
                    .Version = 0,
                };
                if (igclData.ffctlMemoryGetState(memoryModules[iMem], &memoryState) == CTL_RESULT_SUCCESS)
                {
                    result.memory->total += memoryState.size;
                    result.memory->used += memoryState.size - memoryState.free;
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
        uint32_t sensorCount = sizeof(sensors) / sizeof(sensors[0]);
        if (igclData.ffctlEnumTemperatureSensors(device, &sensorCount, sensors) == CTL_RESULT_SUCCESS)
        {
            double sumValue = 0;
            uint32_t availableCount = 0;
            for (uint32_t iSensor = 0; iSensor < sensorCount; iSensor++)
            {
                double value;
                if (igclData.ffctlTemperatureGetState(sensors[iSensor], &value) == CTL_RESULT_SUCCESS)
                {
                    sumValue += value;
                    availableCount++;
                }
            }
            *result.temp = sumValue / availableCount;
        }
    }

    return NULL;
}
