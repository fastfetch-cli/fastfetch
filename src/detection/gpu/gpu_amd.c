#include "gpu_driver_specific.h"

#include "adl.h"
#include "common/library.h"
#include "util/mallocHelper.h"
#include "util/debug.h"

// Helper function to convert ADL status code to string
FF_MAYBE_UNUSED static const char* ffAdlStatusToString(int status) {
    switch (status) {
        #define FF_ADL_STATUS_CASE(name) case name: return #name;
        FF_ADL_STATUS_CASE(ADL_OK)
        FF_ADL_STATUS_CASE(ADL_OK_WARNING)
        FF_ADL_STATUS_CASE(ADL_OK_MODE_CHANGE)
        FF_ADL_STATUS_CASE(ADL_OK_RESTART)
        FF_ADL_STATUS_CASE(ADL_OK_WAIT)
        FF_ADL_STATUS_CASE(ADL_ERR)
        FF_ADL_STATUS_CASE(ADL_ERR_NOT_INIT)
        FF_ADL_STATUS_CASE(ADL_ERR_INVALID_PARAM)
        FF_ADL_STATUS_CASE(ADL_ERR_INVALID_PARAM_SIZE)
        FF_ADL_STATUS_CASE(ADL_ERR_INVALID_ADL_IDX)
        FF_ADL_STATUS_CASE(ADL_ERR_INVALID_CONTROLLER_IDX)
        FF_ADL_STATUS_CASE(ADL_ERR_INVALID_DIPLAY_IDX)
        FF_ADL_STATUS_CASE(ADL_ERR_NOT_SUPPORTED)
        FF_ADL_STATUS_CASE(ADL_ERR_NULL_POINTER)
        FF_ADL_STATUS_CASE(ADL_ERR_DISABLED_ADAPTER)
        FF_ADL_STATUS_CASE(ADL_ERR_INVALID_CALLBACK)
        FF_ADL_STATUS_CASE(ADL_ERR_RESOURCE_CONFLICT)
        FF_ADL_STATUS_CASE(ADL_ERR_SET_INCOMPLETE)
        FF_ADL_STATUS_CASE(ADL_ERR_NO_XDISPLAY)
        FF_ADL_STATUS_CASE(ADL_ERR_CALL_TO_INCOMPATIABLE_DRIVER)
        FF_ADL_STATUS_CASE(ADL_ERR_NO_ADMINISTRATOR_PRIVILEGES)
        FF_ADL_STATUS_CASE(ADL_ERR_FEATURESYNC_NOT_STARTED)
        FF_ADL_STATUS_CASE(ADL_ERR_INVALID_POWER_STATE)
        #undef FF_ADL_STATUS_CASE
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
    FF_LIBRARY_SYMBOL(ADL2_Adapter_ASICFamilyType_Get)
    FF_LIBRARY_SYMBOL(ADL2_Overdrive_Caps)
    FF_LIBRARY_SYMBOL(ADL2_OverdriveN_CapabilitiesX2_Get)
    FF_LIBRARY_SYMBOL(ADL2_OverdriveN_SystemClocksX2_Get)
    FF_LIBRARY_SYMBOL(ADL2_OverdriveN_PerformanceStatus_Get)
    FF_LIBRARY_SYMBOL(ADL2_OverdriveN_Temperature_Get)
    FF_LIBRARY_SYMBOL(ADL2_Overdrive8_Current_Setting_Get)
    FF_LIBRARY_SYMBOL(ADL2_New_QueryPMLogData_Get)
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
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_ASICFamilyType_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Overdrive_Caps)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_OverdriveN_CapabilitiesX2_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_OverdriveN_SystemClocksX2_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_OverdriveN_PerformanceStatus_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Overdrive8_Current_Setting_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_New_QueryPMLogData_Get)
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
            FF_DEBUG("Core info - NumCUs: %d, NumPEsPerCU: %d", coreInfo.iNumCUs, coreInfo.iNumPEsPerCU);
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

        if (status == ADL_OK && vramUsage >= 0) {
            result.memory->used = (uint64_t) vramUsage * 1024 * 1024;
            FF_DEBUG("Dedicated VRAM usage: %llu bytes (%d MB)", result.memory->used, vramUsage);
        } else {
            FF_DEBUG("Failed to get dedicated VRAM usage");
        }
    }

    if (result.memoryType)
    {
        ADLMemoryInfo2 memoryInfo;
        int status = adlData.ffADL2_Adapter_MemoryInfo2_Get(adlData.apiHandle, device->iAdapterIndex, &memoryInfo);
        FF_DEBUG("ADL2_Adapter_MemoryInfo2_Get returned %s (%d)", ffAdlStatusToString(status), status);

        if (status == ADL_OK)
        {
            FF_DEBUG("Memory info - Type: %s, Size: %lld MB", memoryInfo.strMemoryType, memoryInfo.iMemorySize / 1024 / 1024);
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
            FF_DEBUG("GPU type: %s (asicTypes: 0x%x, valids: 0x%x)",
                    *result.type == FF_GPU_TYPE_INTEGRATED ? "Integrated" : "Discrete", asicTypes, valids);
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
        FF_DEBUG("Setting adapter name: %s; UDID: %s, Present: %d, Exist: %d", device->strAdapterName, device->strUDID, device->iPresent, device->iExist);
    }

    int odVersion = 0;

    {
        int odSupported = 0;
        int odEnabled = 0;
        int status = adlData.ffADL2_Overdrive_Caps(adlData.apiHandle, device->iAdapterIndex, &odSupported, &odEnabled, &odVersion);
        FF_DEBUG("ADL2_Overdrive_Caps returned %s (%d); supported %d, enabled %d; version %d",
                ffAdlStatusToString(status), status, odSupported, odEnabled, odVersion);
        if (status != ADL_OK)
        {
            FF_DEBUG("Overdrive not supported, results may be inaccurate");
            // Note even if Overdrive is not supported, we can still get the OD version
        }
    }


    if (odVersion == 8)
    {
        FF_DEBUG("Using Overdrive8 API (odVersion=%d)", odVersion);

        if (result.frequency)
        {
            ADLOD8CurrentSetting currentSetting = { .count = OD8_COUNT };
            int status = adlData.ffADL2_Overdrive8_Current_Setting_Get(adlData.apiHandle, device->iAdapterIndex, &currentSetting);
            FF_DEBUG("ADL2_Overdrive8_Current_Setting_Get returned %s (%d)", ffAdlStatusToString(status), status);
            if (status == ADL_OK)
            {
                FF_DEBUG("OD8 Settings count: %d", currentSetting.count);

                *result.frequency = (uint32_t) currentSetting.Od8SettingTable[OD8_GFXCLK_FMAX];
                FF_DEBUG("Got max engine clock (OD8_GFXCLK_FMAX): %u MHz", *result.frequency);
            }
            else
            {
                FF_DEBUG("Failed to get max frequency information");
            }
        }

        if (result.temp || result.coreUsage)
        {
            ADLPMLogDataOutput pmLogDataOutput = {};
            int status = adlData.ffADL2_New_QueryPMLogData_Get(adlData.apiHandle, device->iAdapterIndex, &pmLogDataOutput);
            FF_DEBUG("ADL2_New_QueryPMLogData_Get returned %s (%d)", ffAdlStatusToString(status), status);
            if (status == ADL_OK)
            {
                if (result.temp)
                {
                    ADLSingleSensorData* sensor = &pmLogDataOutput.sensors[ADL_PMLOG_TEMPERATURE_HOTSPOT];
                    FF_DEBUG("Sensor %d: %s, supported: %d, value: %d", ADL_PMLOG_TEMPERATURE_HOTSPOT, "ADL_PMLOG_TEMPERATURE_HOTSPOT", sensor->supported, sensor->value);
                    if (sensor->supported)
                    {
                        *result.temp = sensor->value;
                        FF_DEBUG("Temperature: %.1f°C (HOTSPOT)", *result.temp);
                    }
                    else
                    {
                        sensor = &pmLogDataOutput.sensors[ADL_PMLOG_TEMPERATURE_GFX];
                        FF_DEBUG("Sensor %d: %s, supported: %d, value: %d", ADL_PMLOG_TEMPERATURE_GFX, "ADL_PMLOG_TEMPERATURE_GFX", sensor->supported, sensor->value);
                        if (sensor->supported)
                        {
                            *result.temp = sensor->value;
                            FF_DEBUG("Temperature: %.1f°C (GFX)", *result.temp);
                        }
                        else
                        {
                            sensor = &pmLogDataOutput.sensors[ADL_PMLOG_TEMPERATURE_SOC];
                            FF_DEBUG("Sensor %d: %s, supported: %d, value: %d", ADL_PMLOG_TEMPERATURE_SOC, "ADL_PMLOG_TEMPERATURE_SOC", sensor->supported, sensor->value);
                            if (sensor->supported)
                            {
                                *result.temp = sensor->value;
                                FF_DEBUG("Temperature: %.1f°C (SOC)", *result.temp);
                            }
                            else
                            {
                                FF_DEBUG("No supported temp sensor found, temp detection failed");
                            }
                        }
                    }
                }
                if (result.coreUsage)
                {
                    ADLSingleSensorData* activity = &pmLogDataOutput.sensors[ADL_PMLOG_INFO_ACTIVITY_GFX];
                    FF_DEBUG("Sensor %d: %s, supported: %d, value: %d", ADL_PMLOG_INFO_ACTIVITY_GFX, "ADL_PMLOG_INFO_ACTIVITY_GFX", activity->supported, activity->value);
                    if (activity->supported)
                    {
                        *result.coreUsage = activity->value;
                        FF_DEBUG("Core usage: %.1f%%", *result.coreUsage);
                    }
                    else
                    {
                        FF_DEBUG("Sensor %d not supported, GPU usage detection failed", ADL_PMLOG_INFO_ACTIVITY_GFX);
                    }
                }
            }
            else
            {
                FF_DEBUG("Failed to get temperature / GPU activity");
            }
        }
    }
    else if (odVersion == 7)
    {
        FF_DEBUG("Using OverdriveN API (odVersion=%d)", odVersion);

        if (result.frequency)
        {
            // https://github.com/MaynardMiner/odvii/blob/master/OverdriveN.cpp#L176
            ADLODNCapabilitiesX2 odCapabilities = {};
            int status = adlData.ffADL2_OverdriveN_CapabilitiesX2_Get(adlData.apiHandle, device->iAdapterIndex, &odCapabilities);
            FF_DEBUG("ADL2_OverdriveN_CapabilitiesX2_Get returned %s (%d)", ffAdlStatusToString(status), status);

            if (status == ADL_OK)
            {
                if (odCapabilities.iMaximumNumberOfPerformanceLevels == 0)
                {
                    FF_DEBUG("ADL2_OverdriveN_CapabilitiesX2_Get: no performance levels available");
                }
                else
                {
                    FF_DEBUG("ODN Capabilities - MaxPerformanceLevels: %d, GPU Clock Range: [%d - %d]",
                            odCapabilities.iMaximumNumberOfPerformanceLevels,
                            odCapabilities.sEngineClockRange.iMin, odCapabilities.sEngineClockRange.iMax);

                    size_t size = sizeof(ADLODNPerformanceLevelsX2) + sizeof(ADLODNPerformanceLevelX2) * ((unsigned) odCapabilities.iMaximumNumberOfPerformanceLevels - 1);
                    FF_AUTO_FREE ADLODNPerformanceLevelsX2* odPerfLevels = calloc(size, 1);
                    odPerfLevels->iSize = (int) size;
                    odPerfLevels->iNumberOfPerformanceLevels = odCapabilities.iMaximumNumberOfPerformanceLevels;
                    odPerfLevels->iMode = ODNControlType_Current;

                    int status = adlData.ffADL2_OverdriveN_SystemClocksX2_Get(adlData.apiHandle, device->iAdapterIndex, odPerfLevels);
                    FF_DEBUG("ADL2_OverdriveN_SystemClocksX2_Get returned %s (%d), levels: %d",
                            ffAdlStatusToString(status), status, odPerfLevels->iNumberOfPerformanceLevels);

                    if (status != ADL_OK)
                    {
                        FF_DEBUG("Failed to get frequency information");
                    }
                    else
                    {
                        // lowest to highest
                        for (int i = odPerfLevels->iNumberOfPerformanceLevels - 1; i >= 0 ; i--)
                        {
                            ADLODNPerformanceLevelX2* level = &odPerfLevels->aLevels[i];
                            FF_DEBUG("Performance level %d: enabled: %d, engine clock = %d", i, level->iEnabled, level->iClock);
                            if (level->iEnabled)
                            {
                                *result.frequency = (uint32_t) level->iClock / 100; // in 10 kHz
                                FF_DEBUG("Got max engine clock: %u MHz", *result.frequency);
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                FF_DEBUG("Failed to get frequency information");
            }
        }

        if (result.coreUsage)
        {
            ADLODNPerformanceStatus performanceStatus = {};
            int status = adlData.ffADL2_OverdriveN_PerformanceStatus_Get(adlData.apiHandle, device->iAdapterIndex, &performanceStatus);
            FF_DEBUG("ADL2_OverdriveN_PerformanceStatus_Get returned %s (%d)", ffAdlStatusToString(status), status);

            if (status == ADL_OK)
            {
                FF_DEBUG("Performance Status - Activity: %d%%, CoreClock: %dMHz, MemoryClock: %dMHz",
                        performanceStatus.iGPUActivityPercent,
                        performanceStatus.iCoreClock,
                        performanceStatus.iMemoryClock);

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
            FF_DEBUG("ADL2_OverdriveN_Temperature_Get returned %s (%d)", ffAdlStatusToString(status), status);

            if (status == ADL_OK)
            {
                *result.temp = milliDegrees / 1000.0;
                FF_DEBUG("Temperature: %.1f°C (raw: %d milliC)", *result.temp, milliDegrees);
            }
            else
            {
                FF_DEBUG("Failed to get temperature");
            }
        }
    }
    else if (odVersion == 6)
    {
        FF_DEBUG("Using Overdrive6 API (odVersion=%d)", odVersion);

        if (result.frequency)
        {
            FF_AUTO_FREE ADLOD6StateInfo* stateInfo = calloc(sizeof(ADLOD6StateInfo) + sizeof(ADLOD6PerformanceLevel), 1);
            stateInfo->iNumberOfPerformanceLevels = 2;

            int status = adlData.ffADL2_Overdrive6_StateInfo_Get(adlData.apiHandle, device->iAdapterIndex, ADL_OD6_GETSTATEINFO_CUSTOM_PERFORMANCE, stateInfo);
            FF_DEBUG("ADL2_Overdrive6_StateInfo_Get returned %s (%d), performance levels: %d",
                ffAdlStatusToString(status), status, stateInfo->iNumberOfPerformanceLevels);

            if (status == ADL_OK)
            {
                // OD6 uses clock ranges instead of discrete performance levels.
                // iNumberOfPerformanceLevels is always 2.
                // The 1st level indicates the minimum clocks in the range.
                // The 2nd level indicates the maximum clocks in the range.
                if (stateInfo->iNumberOfPerformanceLevels != 2)
                {
                    FF_DEBUG("ADL2_Overdrive6_StateInfo_Get: unexpected number of performance levels: %d", stateInfo->iNumberOfPerformanceLevels);
                }
                else
                {
                    FF_DEBUG("OD6 Settings - MinPerformanceLevels: %d, MaxPerformanceLevels: %d",
                            stateInfo->aLevels[0].iEngineClock, stateInfo->aLevels[1].iEngineClock);
                    *result.frequency = (uint32_t) stateInfo->aLevels[1].iEngineClock / 100; // in 10 kHz
                    FF_DEBUG("Got max engine clock: %u MHz", *result.frequency);
                }
            }
            else
            {
                FF_DEBUG("Failed to get frequency information");
            }
        }

        if (result.coreUsage)
        {
            ADLOD6CurrentStatus status = {};
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
    else
    {
        FF_DEBUG("Unknown Overdrive version: %d", odVersion);
        return "Unknown Overdrive version";
    }
    FF_DEBUG("AMD GPU detection complete - returning success");
    return NULL;
}
