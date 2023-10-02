#include "cpu.h"
#include "detection/temps/temps_windows.h"
#include "util/windows/registry.h"
#include "util/mallocHelper.h"

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    {
        DWORD length = 0;
        GetLogicalProcessorInformationEx(RelationAll, NULL, &length);
        if (length == 0)
            return "GetLogicalProcessorInformationEx(RelationAll, NULL, &length) failed";

        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* FF_AUTO_FREE
            pProcessorInfo = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(length);

        if (pProcessorInfo && GetLogicalProcessorInformationEx(RelationAll, pProcessorInfo, &length))
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
        else
            return "GetLogicalProcessorInformationEx(RelationAll, pProcessorInfo, &length) failed";
    }

    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hKey, NULL))
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\", &hKey, NULL) failed";

    {
        uint32_t mhz;
        if(ffRegReadUint(hKey, L"~MHz", &mhz, NULL))
            cpu->frequencyMax = mhz / 1000.0;
    }

    ffRegReadStrbuf(hKey, L"ProcessorNameString", &cpu->name, NULL);
    ffRegReadStrbuf(hKey, L"VendorIdentifier", &cpu->vendor, NULL);

    if(options->temp)
        ffDetectSmbiosTemp(&cpu->temperature, NULL);

    return NULL;
}
