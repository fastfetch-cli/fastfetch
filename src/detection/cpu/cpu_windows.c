#include "cpu.h"
#include "detection/temps/temps_windows.h"
#include "util/windows/registry.h"
#include "util/mallocHelper.h"

void ffDetectCPUImpl(const FFinstance* instance, FFCPUResult* cpu)#include "cpu.h"
#include "detection/temps/temps_windows.h"
#include "util/windows/registry.h"
#include "util/mallocHelper.h"

void ffDetectCPUImpl(const FFinstance* instance, FFCPUResult* cpu)
{
    FF_UNUSED(instance);

    cpu->temperature = FF_CPU_TEMP_UNSET;
    cpu->coresPhysical = cpu->coresLogical = cpu->coresOnline = 0;
    cpu->frequencyMax = cpu->frequencyMin = 0;
    ffStrbufInit(&cpu->name);
    ffStrbufInit(&cpu->vendor);

    {
        DWORD length = 0;
        GetLogicalProcessorInformationEx(RelationAll, NULL, &length);
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* FF_AUTO_FREE
            pProcessorInfo = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(length);

        if(pProcessorInfo && GetLogicalProcessorInformationEx(RelationAll, pProcessorInfo, &length))
        {
            for(
                SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* ptr = pProcessorInfo;
                (uint8_t*)ptr < ((uint8_t*)pProcessorInfo) + length;
                ptr = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(((uint8_t*)ptr) + ptr->Size)
            )
            {
                if(ptr->Relationship == RelationProcessorCore)
                    ++cpu->coresPhysical;
                else if(ptr->Relationship == RelationGroup)
                {
                    cpu->coresOnline += ptr->Group.GroupInfo->ActiveProcessorCount;
                    cpu->coresLogical += ptr->Group.GroupInfo->MaximumProcessorCount;
                }
            }
        }

        free(pProcessorInfo); // Free the allocated memory
    }

    FF_HKEY_AUTO_DESTROY hKey;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hKey, NULL))
        return;

    {
        uint32_t mhz;
        if(ffRegReadUint(hKey, L"~MHz", &mhz, NULL))
            cpu->frequencyMax = mhz / 1000.0;
    }

    ffRegReadStrbuf(hKey, L"ProcessorNameString", &cpu->name, NULL);
    ffRegReadStrbuf(hKey, L"VendorIdentifier", &cpu->vendor, NULL);

    if(instance->config.cpuTemp)
        ffDetectSmbiosTemp(&cpu->temperature, NULL);
}

{
    FF_UNUSED(instance);

    cpu->temperature = FF_CPU_TEMP_UNSET;
    cpu->coresPhysical = cpu->coresLogical = cpu->coresOnline = 0;
    cpu->frequencyMax = cpu->frequencyMin = 0;
    ffStrbufInit(&cpu->name);
    ffStrbufInit(&cpu->vendor);

    {
        DWORD length = 0;
        GetLogicalProcessorInformationEx(RelationAll, NULL, &length);
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* FF_AUTO_FREE
            pProcessorInfo = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(length);

        if(pProcessorInfo && GetLogicalProcessorInformationEx(RelationAll, pProcessorInfo, &length))
        {
            for(
                SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* ptr = pProcessorInfo;
                (uint8_t*)ptr < ((uint8_t*)pProcessorInfo) + length;
                ptr = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(((uint8_t*)ptr) + ptr->Size)
            )
            {
                if(ptr->Relationship == RelationProcessorCore)
                    ++cpu->coresPhysical;
                else if(ptr->Relationship == RelationGroup)
                {
                    cpu->coresOnline += ptr->Group.GroupInfo->ActiveProcessorCount;
                    cpu->coresLogical += ptr->Group.GroupInfo->MaximumProcessorCount;
                }
            }
        }
    }

    FF_HKEY_AUTO_DESTROY hKey;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hKey, NULL))
        return;

    {
        uint32_t mhz;
        if(ffRegReadUint(hKey, L"~MHz", &mhz, NULL))
            cpu->frequencyMax = mhz / 1000.0;
    }

    ffRegReadStrbuf(hKey, L"ProcessorNameString", &cpu->name, NULL);
    ffRegReadStrbuf(hKey, L"VendorIdentifier", &cpu->vendor, NULL);

    if(instance->config.cpuTemp)
        ffDetectSmbiosTemp(&cpu->temperature, NULL);
}
