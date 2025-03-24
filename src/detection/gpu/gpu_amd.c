#include "gpu_driver_specific.h"

#include "adl.h"
#include "common/library.h"
#include "util/mallocHelper.h"

// Memory allocation function
void* __attribute__((__stdcall__)) ADL_Main_Memory_Alloc(int iSize)
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
        adlData.ffADL2_Main_Control_Destroy(adlData.apiHandle);
        adlData.apiHandle = NULL;
    }
}

const char* ffDetectAmdGpuInfo(const FFGpuDriverCondition* cond, FFGpuDriverResult result, const char* soName)
{
    if (!adlData.inited)
    {
        adlData.inited = true;
        FF_LIBRARY_LOAD(atiadl, "dlopen atiadlxx failed", soName , 1);
        FF_LIBRARY_LOAD_SYMBOL_MESSAGE(atiadl, ADL2_Main_Control_Create)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Main_Control_Destroy)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_AdapterInfoX3_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_Graphic_Core_Info_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_MemoryInfo2_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_DedicatedVRAMUsage_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_VRAMUsage_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Adapter_ASICFamilyType_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Overdrive6_CurrentStatus_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Overdrive6_Temperature_Get)
        FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(atiadl, adlData, ADL2_Overdrive6_StateInfo_Get)

        if (ffADL2_Main_Control_Create(ADL_Main_Memory_Alloc, 1 /*iEnumConnectedAdapters*/, &adlData.apiHandle) != ADL_OK)
            return "ffADL2_Main_Control_Create() failed";

        atexit(shutdownAdl);
        atiadl = NULL; // don't close atiadl
    }

    if (!adlData.apiHandle)
        return "ffADL2_Main_Control_Create() failed";

    FF_AUTO_FREE AdapterInfo* devices = NULL;
    int numDevices = 0;
    if (adlData.ffADL2_Adapter_AdapterInfoX3_Get(adlData.apiHandle, -1, &numDevices, &devices) == 0)
        return "ffADL2_Adapter_AdapterInfoX3_Get() failed";

    const AdapterInfo* device = NULL;
    for (int iDev = 0; iDev < numDevices; iDev++)
    {
        if (cond->type & FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID)
        {
            if (
                cond->pciBusId.bus == (uint32_t) devices[iDev].iBusNumber &&
                cond->pciBusId.device == (uint32_t) devices[iDev].iDeviceNumber &&
                cond->pciBusId.func == (uint32_t) devices[iDev].iFunctionNumber)
            {
                device = &devices[iDev];
                break;
            }
        }
    }

    if (!device)
        return "Device not found";

    if (result.coreCount)
    {
        ADLGraphicCoreInfo coreInfo;
        if (adlData.ffADL2_Adapter_Graphic_Core_Info_Get(adlData.apiHandle, device->iAdapterIndex, &coreInfo) == ADL_OK)
            *result.coreCount = (uint32_t) coreInfo.iNumCUs;
    }

    if (result.memory)
    {
        int vramUsage = 0;
        if (adlData.ffADL2_Adapter_DedicatedVRAMUsage_Get(adlData.apiHandle, device->iAdapterIndex, &vramUsage) == ADL_OK)
            result.memory->used = (uint64_t) vramUsage * 1024 * 1024;
        if (result.sharedMemory)
        {
            vramUsage = 0;
            if (adlData.ffADL2_Adapter_VRAMUsage_Get(adlData.apiHandle, device->iAdapterIndex, &vramUsage) == ADL_OK)
                result.sharedMemory->used = (uint64_t) vramUsage * 1024 * 1024 - result.memory->used;
        }
    }

    if (result.memoryType)
    {
        ADLMemoryInfo2 memoryInfo;
        if (adlData.ffADL2_Adapter_MemoryInfo2_Get(adlData.apiHandle, device->iAdapterIndex, &memoryInfo) == ADL_OK)
            ffStrbufSetS(result.memoryType, memoryInfo.strMemoryType);
    }

    if (result.frequency)
    {
        ADLOD6StateInfo stateInfo;
        if (adlData.ffADL2_Overdrive6_StateInfo_Get(adlData.apiHandle, device->iAdapterIndex, ADL_OD6_GETSTATEINFO_CUSTOM_PERFORMANCE, &stateInfo) == ADL_OK)
        {
            int clock = 0; // assume in 10 kHz
            for (int i = 0; i < stateInfo.iNumberOfPerformanceLevels; i++)
            {
                if (stateInfo.aLevels[i].iEngineClock > clock)
                    clock = stateInfo.aLevels[i].iEngineClock;
            }
            *result.frequency = (uint32_t) clock / 100;
        }
    }

    if (result.coreUsage)
    {
        ADLOD6CurrentStatus status;
        if (adlData.ffADL2_Overdrive6_CurrentStatus_Get(adlData.apiHandle, device->iAdapterIndex, &status) == ADL_OK)
        {
            if (result.coreUsage)
                *result.coreUsage = status.iActivityPercent;
        }
    }

    if (result.type)
    {
        int asicTypes = 0;
        int valids = 0;
        if (adlData.ffADL2_Adapter_ASICFamilyType_Get(adlData.apiHandle, device->iAdapterIndex, &asicTypes, &valids) == ADL_OK)
        {
            asicTypes &= valids; // This design is strange
            *result.type = asicTypes & ADL_ASIC_INTEGRATED ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;
        }
    }

    if (result.index)
        *result.index = (uint32_t) device->iAdapterIndex;

    if (result.name)
        ffStrbufSetS(result.name, device->strAdapterName);

    if (result.temp)
    {
        int milliDegrees = 0;
        if (adlData.ffADL2_Overdrive6_Temperature_Get(adlData.apiHandle, device->iAdapterIndex, &milliDegrees) == ADL_OK)
            *result.temp = milliDegrees / 1000.0;
    }

    return NULL;
}
