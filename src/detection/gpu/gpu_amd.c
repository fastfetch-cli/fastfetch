#include "gpu_driver_specific.h"

#include "adl.h"
#include "common/library.h"
#include "util/mallocHelper.h"
#include "util/debug.h"

// Helper function to convert ADL status code to string
static const char* ffAdlStatusToString(int status) {
    switch (status) {
        case ADL_OK: return "ADL_OK";
        case ADL_OK_WARNING: return "ADL_OK_WARNING";
        case ADL_OK_MODE_CHANGE: return "ADL_OK_MODE_CHANGE";
        case ADL_OK_RESTART: return "ADL_OK_RESTART";
        case ADL_OK_WAIT: return "ADL_OK_WAIT";
        case ADL_ERR: return "ADL_ERR";
        case ADL_ERR_NOT_INIT: return "ADL_ERR_NOT_INIT";
        case ADL_ERR_INVALID_PARAM: return "ADL_ERR_INVALID_PARAM";
        case ADL_ERR_INVALID_PARAM_SIZE: return "ADL_ERR_INVALID_PARAM_SIZE";
        case ADL_ERR_INVALID_ADL_IDX: return "ADL_ERR_INVALID_ADL_IDX";
        case ADL_ERR_INVALID_CONTROLLER_IDX: return "ADL_ERR_INVALID_CONTROLLER_IDX";
        case ADL_ERR_INVALID_DIPLAY_IDX: return "ADL_ERR_INVALID_DIPLAY_IDX";
        case ADL_ERR_NOT_SUPPORTED: return "ADL_ERR_NOT_SUPPORTED";
        case ADL_ERR_NULL_POINTER: return "ADL_ERR_NULL_POINTER";
        case ADL_ERR_DISABLED_ADAPTER: return "ADL_ERR_DISABLED_ADAPTER";
        case ADL_ERR_INVALID_CALLBACK: return "ADL_ERR_INVALID_CALLBACK";
        case ADL_ERR_RESOURCE_CONFLICT: return "ADL_ERR_RESOURCE_CONFLICT";
        case ADL_ERR_SET_INCOMPLETE: return "ADL_ERR_SET_INCOMPLETE";
        case ADL_ERR_NO_XDISPLAY: return "ADL_ERR_NO_XDISPLAY";
        case ADL_ERR_CALL_TO_INCOMPATIABLE_DRIVER: return "ADL_ERR_CALL_TO_INCOMPATIABLE_DRIVER";
        case ADL_ERR_NO_ADMINISTRATOR_PRIVILEGES: return "ADL_ERR_NO_ADMINISTRATOR_PRIVILEGES";
        case ADL_ERR_FEATURESYNC_NOT_STARTED: return "ADL_ERR_FEATURESYNC_NOT_STARTED";
        case ADL_ERR_INVALID_POWER_STATE: return "ADL_ERR_INVALID_POWER_STATE";
        default: return "Unknown ADL error";
    }
}

// Memory allocation function
static void* __attribute__((__stdcall__)) ffAdlMainMemoryAlloc(int iSize)
{
    return malloc((size_t) iSize);
}

struct FFAdlData {
    FF_LIBRARY_SYMBOL(ADL2_Main_Control_Destroy)
    FF_LIBRARY_SYMBOL(ADL2_Adapter_AdapterInfoX3_Get)
    FF_LIBRARY_SYMBOL(ADL2_Adapter_Graphic_Core_Info_Get)
    FF_LIBRARY_SYMBOL(ADL2_Adapter_MemoryInfo2_Get)
    FF_LIBRARY_SYMBOL(ADL2_Adapter_DedicatedVRAMUsage_Get)
    FF_LIBRARY_SYMBOL(ADL2_Adapter_VRAMUsage_Get)
    FF_LIBRARY_SYMBOL(ADL2_Adapter_ASICFamilyType_Get)
    FF_LIBRARY_SYMBOL(ADL2_Overdrive_Caps)
    FF_LIBRARY_SYMBOL(ADL2_OverdriveN_Capabilities_Get)
    FF_LIBRARY_SYMBOL(ADL2_OverdriveN_PerformanceStatus_Get)
    FF_LIBRARY_SYMBOL(ADL2_OverdriveN_Temperature_Get)
    FF_LIBRARY_SYMBOL(ADL2_Overdrive6_CurrentStatus_Get)
    FF_LIBRARY_SYMBOL(ADL2_Overdrive6_Temperature_Get)
    FF_LIBRARY_SYMBOL(ADL2_Overdrive6_StateInfo_Get)

    bool inited;
    ADL_CONTEXT_HANDLE apiHandle;
} adlData;

static void shutdownAdl()
{
    if (adlData.apiHandle)
    {
        FF_DEBUG("Destroying ADL context");
        adlData.ffADL2_Main_Control_Destroy(adlData.apiHandle);
        adlData.apiHandle = NULL;
    }
}

const char* ffDetectAmdGpuInfo(const FFGpuDriverCondition* cond, FFGpuDriverResult result, const char* soName)
{
    FF_DEBUG("Attempting to detect AMD GPU info using '%s'", soName);

    if (!adlData.inited)
    {
        adlData.inited = true;
        FF_DEBUG("Initializing ADL library");
        FF_LIBRARY_LOAD(atiadl, "dlopen atiadlxx failed", soName , 1);
        FF_LIBRARY_LOAD_SYMBOL_MESSAGE(atiadl, ADL2_Main_Control_Create)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Main_Control_Destroy)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_AdapterInfoX3_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_Graphic_Core_Info_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_MemoryInfo2_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_DedicatedVRAMUsage_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_VRAMUsage_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_ASICFamilyType_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Overdrive_Caps)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_OverdriveN_Capabilities_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_OverdriveN_PerformanceStatus_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_OverdriveN_Temperature_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Overdrive6_CurrentStatus_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Overdrive6_Temperature_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Overdrive6_StateInfo_Get)
        FF_DEBUG("ADL library loaded");

        int result = ffADL2_Main_Control_Create(ffAdlMainMemoryAlloc, 1 /*iEnumConnectedAdapters*/, &adlData.apiHandle);
        FF_DEBUG("ADL2_Main_Control_Create returned %s (%d)", ffAdlStatusToString(result), result);
        if (result != ADL_OK)
            return "ffADL2_Main_Control_Create() failed";

        atexit(shutdownAdl);
        atiadl = NULL; // don't close atiadl
        FF_DEBUG("ADL initialization complete");
    }

    if (!adlData.apiHandle)
    {
        FF_DEBUG("ADL context not initialized");
        return "ffADL2_Main_Control_Create() failed";
    }

    FF_AUTO_FREE AdapterInfo* devices = NULL;
    int numDevices = 0;
    int adapterResult = adlData.ffADL2_Adapter_AdapterInfoX3_Get(adlData.apiHandle, -1, &numDevices, &devices);
    FF_DEBUG("ADL2_Adapter_AdapterInfoX3_Get returned %s (%d)", ffAdlStatusToString(adapterResult), adapterResult);

    if (adapterResult == ADL_OK)
    {
        FF_DEBUG("found %d adapters", numDevices);
    }
    else
    {
        FF_DEBUG("ffADL2_Adapter_AdapterInfoX3_Get() failed");
        return "ffADL2_Adapter_AdapterInfoX3_Get() failed";
    }

    const AdapterInfo* device = NULL;
    for (int iDev = 0; iDev < numDevices; iDev++)
    {
        if (cond->type & FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID)
        {
            FF_DEBUG("Checking device %d: bus=%d, device=%d, func=%d against requested bus=%u, device=%u, func=%u",
                iDev, devices[iDev].iBusNumber, devices[iDev].iDeviceNumber, devices[iDev].iFunctionNumber,
                cond->pciBusId.bus, cond->pciBusId.device, cond->pciBusId.func);

            if (
                cond->pciBusId.bus == (uint32_t) devices[iDev].iBusNumber &&
                cond->pciBusId.device == (uint32_t) devices[iDev].iDeviceNumber &&
                cond->pciBusId.func == (uint32_t) devices[iDev].iFunctionNumber)
            {
                device = &devices[iDev];
                FF_DEBUG("Found matching device: %s (index: %d)", device->strAdapterName, device->iAdapterIndex);
                break;
            }
        }
    }

    if (!device)
    {
        FF_DEBUG("Device not found");
        return "Device not found";
    }

    if (result.coreCount)
    {
        ADLGraphicCoreInfo coreInfo;
        int status = adlData.ffADL2_Adapter_Graphic_Core_Info_Get(adlData.apiHandle, device->iAdapterIndex, &coreInfo);
        FF_DEBUG("ADL2_Adapter_Graphic_Core_Info_Get returned %s (%d)", ffAdlStatusToString(status), status);

        if (status == ADL_OK)
        {
            *result.coreCount = (uint32_t) coreInfo.iNumCUs * (uint32_t) coreInfo.iNumPEsPerCU;
            FF_DEBUG("Got core count: %u", *result.coreCount);
        }
        else
        {
            FF_DEBUG("Failed to get core count");
        }
    }

    if (result.memory)
    {
        int vramUsage = 0;
        int status = adlData.ffADL2_Adapter_DedicatedVRAMUsage_Get(adlData.apiHandle, device->iAdapterIndex, &vramUsage);
        FF_DEBUG("ADL2_Adapter_DedicatedVRAMUsage_Get returned %s (%d), usage: %d MB",
            ffAdlStatusToString(status), status, vramUsage);

        if (status == ADL_OK)
            result.memory->used = (uint64_t) vramUsage * 1024 * 1024;

        if (result.sharedMemory)
        {
            vramUsage = 0;
            status = adlData.ffADL2_Adapter_VRAMUsage_Get(adlData.apiHandle, device->iAdapterIndex, &vramUsage);
            FF_DEBUG("ADL2_Adapter_VRAMUsage_Get returned %s (%d), usage: %d MB",
                ffAdlStatusToString(status), status, vramUsage);

            if (status == ADL_OK)
                result.sharedMemory->used = (uint64_t) vramUsage * 1024 * 1024 - result.memory->used;
        }
    }

    if (result.memoryType)
    {
        ADLMemoryInfo2 memoryInfo;
        int status = adlData.ffADL2_Adapter_MemoryInfo2_Get(adlData.apiHandle, device->iAdapterIndex, &memoryInfo);
        FF_DEBUG("ADL2_Adapter_MemoryInfo2_Get returned %s (%d)", ffAdlStatusToString(status), status);

        if (status == ADL_OK)
        {
            ffStrbufSetS(result.memoryType, memoryInfo.strMemoryType);
            FF_DEBUG("Got memory type: %s", memoryInfo.strMemoryType);
        }
        else
        {
            FF_DEBUG("Failed to get memory type");
        }
    }

    if (result.type)
    {
        int asicTypes = 0;
        int valids = 0;
        int status = adlData.ffADL2_Adapter_ASICFamilyType_Get(adlData.apiHandle, device->iAdapterIndex, &asicTypes, &valids);
        FF_DEBUG("ADL2_Adapter_ASICFamilyType_Get returned %s (%d), asicTypes: 0x%x, valids: 0x%x",
            ffAdlStatusToString(status), status, asicTypes, valids);

        if (status == ADL_OK)
        {
            asicTypes &= valids; // This design is strange
            *result.type = asicTypes & ADL_ASIC_INTEGRATED ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;
            FF_DEBUG("GPU type: %s", *result.type == FF_GPU_TYPE_INTEGRATED ? "Integrated" : "Discrete");
        }
        else
        {
            FF_DEBUG("Failed to get GPU type");
        }
    }

    if (result.index)
    {
        *result.index = (uint32_t) device->iAdapterIndex;
        FF_DEBUG("Setting adapter index: %u", *result.index);
    }

    if (result.name)
    {
        ffStrbufSetS(result.name, device->strAdapterName);
        FF_DEBUG("Setting adapter name: %s", device->strAdapterName);
    }

    int odVersion = 0;

    {
        int odSupported = 0;
        int odEnabled = 0;
        int status = adlData.ffADL2_Overdrive_Caps(adlData.apiHandle, device->iAdapterIndex, &odSupported, &odEnabled, &odVersion);
        FF_DEBUG("ADL2_Overdrive_Caps returned %s (%d); supported %d, enabled %d; version %d", ffAdlStatusToString(status), status, odSupported, odEnabled, odVersion);
    }

    if (odVersion >= 6)
    {
        FF_DEBUG("Using OverdriveN API (odVersion=%d)", odVersion);

        if (result.frequency)
        {
            ADLODNCapabilities odCapabilities;
            int status = adlData.ffADL2_OverdriveN_Capabilities_Get(adlData.apiHandle, device->iAdapterIndex, &odCapabilities);
            FF_DEBUG("ADL2_OverdriveN_Capabilities_Get returned %s (%d)", ffAdlStatusToString(status), status);

            if (status == ADL_OK)
            {
                *result.frequency = (uint32_t) odCapabilities.sEngineClockRange.iMax / 100; // assume in 10 KHz
                FF_DEBUG("Got max engine clock: %u MHz", *result.frequency);
            }
            else
            {
                FF_DEBUG("Failed to get frequency information");
            }
        }

        if (result.coreUsage)
        {
            ADLODNPerformanceStatus performanceStatus;
            int status = adlData.ffADL2_OverdriveN_PerformanceStatus_Get(adlData.apiHandle, device->iAdapterIndex, &performanceStatus);
            FF_DEBUG("ADL2_OverdriveN_PerformanceStatus_Get returned %s (%d)", ffAdlStatusToString(status), status);

            if (status == ADL_OK)
            {
                *result.coreUsage = performanceStatus.iGPUActivityPercent;
                FF_DEBUG("Got GPU activity: %d%%", performanceStatus.iGPUActivityPercent);
            }
            else
            {
                FF_DEBUG("Failed to get GPU activity");
            }
        }

        if (result.temp)
        {
            int milliDegrees = 0;
            int status = adlData.ffADL2_OverdriveN_Temperature_Get(adlData.apiHandle, device->iAdapterIndex, 1, &milliDegrees);
            FF_DEBUG("ADL2_OverdriveN_Temperature_Get returned %s (%d), temperature: %d milliC",
                ffAdlStatusToString(status), status, milliDegrees);

            if (status == ADL_OK)
            {
                *result.temp = milliDegrees / 1000.0;
                FF_DEBUG("Temperature: %.1f°C", *result.temp);
            }
            else
            {
                FF_DEBUG("Failed to get temperature");
            }
        }
    }
    else
    {
        FF_DEBUG("Using Overdrive6 API (odVersion=%d)", odVersion);

        if (result.frequency)
        {
            ADLOD6StateInfo stateInfo;
            int status = adlData.ffADL2_Overdrive6_StateInfo_Get(adlData.apiHandle, device->iAdapterIndex, ADL_OD6_GETSTATEINFO_CUSTOM_PERFORMANCE, &stateInfo);
            FF_DEBUG("ADL2_Overdrive6_StateInfo_Get returned %s (%d), performance levels: %d",
                ffAdlStatusToString(status), status, stateInfo.iNumberOfPerformanceLevels);

            if (status == ADL_OK)
            {
                int clock = 0; // assume in 10 kHz
                for (int i = 0; i < stateInfo.iNumberOfPerformanceLevels; i++)
                {
                    FF_DEBUG("Performance level %d: engine clock = %d", i, stateInfo.aLevels[i].iEngineClock);
                    if (stateInfo.aLevels[i].iEngineClock > clock)
                        clock = stateInfo.aLevels[i].iEngineClock;
                }
                *result.frequency = (uint32_t) clock / 100;
                FF_DEBUG("Using max engine clock: %u MHz", *result.frequency);
            }
            else
            {
                FF_DEBUG("Failed to get frequency information");
            }
        }

        if (result.coreUsage)
        {
            ADLOD6CurrentStatus status;
            int apiStatus = adlData.ffADL2_Overdrive6_CurrentStatus_Get(adlData.apiHandle, device->iAdapterIndex, &status);
            FF_DEBUG("ADL2_Overdrive6_CurrentStatus_Get returned %s (%d)", ffAdlStatusToString(apiStatus), apiStatus);

            if (apiStatus == ADL_OK)
            {
                *result.coreUsage = status.iActivityPercent;
                FF_DEBUG("Got GPU activity: %d%%", status.iActivityPercent);
            }
            else
            {
                FF_DEBUG("Failed to get GPU activity");
            }
        }

        if (result.temp)
        {
            int milliDegrees = 0;
            int status = adlData.ffADL2_Overdrive6_Temperature_Get(adlData.apiHandle, device->iAdapterIndex, &milliDegrees);
            FF_DEBUG("ADL2_Overdrive6_Temperature_Get returned %s (%d), temperature: %d milliC",
                ffAdlStatusToString(status), status, milliDegrees);

            if (status == ADL_OK)
            {
                *result.temp = milliDegrees / 1000.0;
                FF_DEBUG("Temperature: %.1f°C", *result.temp);
            }
            else
            {
                FF_DEBUG("Failed to get temperature");
            }
        }
    }
    FF_DEBUG("AMD GPU detection complete");
    return NULL;
}
