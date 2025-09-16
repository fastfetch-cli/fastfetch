#include "cpu.h"
#include "util/windows/registry.h"
#include "util/windows/nt.h"
#include "util/mallocHelper.h"
#include "util/smbiosHelper.h"

#include <windows.h>
#include "util/windows/perflib_.h"
#include <wchar.h>

static inline void ffPerfCloseQueryHandle(HANDLE* phQuery)
{
    if (*phQuery != NULL)
    {
        PerfCloseQueryHandle(*phQuery);
        *phQuery = NULL;
    }
}

const char* detectThermalTemp(double* result)
{
    struct FFPerfQuerySpec
    {
        PERF_COUNTER_IDENTIFIER Identifier;
        WCHAR Name[16];
    } querySpec = {
        .Identifier = {
            // Thermal Zone Information
            // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Perflib\_V2Providers\{383487a6-3676-4870-a4e7-d45b30c35629}\{52bc5412-dac2-449c-8bc2-96443888fe6b}
            .CounterSetGuid = { 0x52bc5412, 0xdac2, 0x449c, {0x8b, 0xc2, 0x96, 0x44, 0x38, 0x88, 0xfe, 0x6b} },
            .Size = sizeof(querySpec),
            .CounterId = PERF_WILDCARD_COUNTER,
            .InstanceId = PERF_WILDCARD_COUNTER,
        },
        .Name = L"\\_TZ.CPUZ", // The standard(?) instance name for CPU temperature in the thermal provider
    };

    DWORD dataSize = 0;
    if (PerfEnumerateCounterSetInstances(NULL, &querySpec.Identifier.CounterSetGuid, NULL, 0, &dataSize) != ERROR_NOT_ENOUGH_MEMORY)
        return "PerfEnumerateCounterSetInstances() failed";

    if (dataSize <= sizeof(PERF_INSTANCE_HEADER))
        return "No `Thermal Zone Information` instances found";

    {
        FF_AUTO_FREE PERF_INSTANCE_HEADER* const pHead = malloc(dataSize);
        if (PerfEnumerateCounterSetInstances(NULL, &querySpec.Identifier.CounterSetGuid, pHead, dataSize, &dataSize) != ERROR_SUCCESS)
            return "PerfEnumerateCounterSetInstances() failed to get instance headers";

        PERF_INSTANCE_HEADER* pInstanceHeader = pHead;
        while (1)
        {
            const wchar_t* instanceName = (const wchar_t*)((BYTE*)pInstanceHeader + sizeof(*pInstanceHeader));
            if (wcscmp(instanceName, querySpec.Name) == 0)
                break;

            dataSize -= pInstanceHeader->Size;
            if (dataSize == 0)
                break;
            pInstanceHeader = (PERF_INSTANCE_HEADER*)((BYTE*)pInstanceHeader + pInstanceHeader->Size);
        }

        if (dataSize == 0)
        {
            const wchar_t* instanceName = (const wchar_t*)((BYTE*)pHead + sizeof(*pHead));
            wcscpy(querySpec.Name, instanceName); // Use the first instance name if the specific one is not found
        }
    }

    __attribute__((__cleanup__(ffPerfCloseQueryHandle)))
    HANDLE hQuery = NULL;

    if (PerfOpenQueryHandle(NULL, &hQuery) != ERROR_SUCCESS)
        return "PerfOpenQueryHandle() failed";

    if (PerfAddCounters(hQuery, &querySpec.Identifier, sizeof(querySpec)) != ERROR_SUCCESS)
        return "PerfAddCounters() failed";

    if (querySpec.Identifier.Status != ERROR_SUCCESS)
        return "PerfAddCounters() reports invalid identifier";

    if (PerfQueryCounterData(hQuery, NULL, 0, &dataSize) != ERROR_NOT_ENOUGH_MEMORY)
        return "PerfQueryCounterData(NULL) failed";

    if (dataSize <= sizeof(PERF_DATA_HEADER) + sizeof(PERF_COUNTER_HEADER)) // PERF_ERROR_RETURN, should not happen
        return "instance doesn't exist";

    FF_AUTO_FREE PERF_DATA_HEADER* const pDataHeader = malloc(dataSize);

    if (PerfQueryCounterData(hQuery, pDataHeader, dataSize, &dataSize) != ERROR_SUCCESS)
        return "PerfQueryCounterData(pDataHeader) failed";

    PERF_COUNTER_HEADER* pCounterHeader = (PERF_COUNTER_HEADER*)(pDataHeader + 1);
    if (pCounterHeader->dwType != PERF_MULTIPLE_COUNTERS)
        return "Invalid counter type";

    PERF_MULTI_COUNTERS* pMultiCounters = (PERF_MULTI_COUNTERS*)(pCounterHeader + 1);
    PERF_COUNTER_DATA* pCounterData = (PERF_COUNTER_DATA*)((BYTE*)pMultiCounters + pMultiCounters->dwSize);

    for (ULONG iCounter = 0; iCounter != pMultiCounters->dwCounters; iCounter++)
    {
        if (pCounterData->dwDataSize == sizeof(int32_t))
        {
            DWORD* pCounterIds = (DWORD*)(pMultiCounters + 1);
            int32_t value = *(int32_t*)(pCounterData + 1);
            if (value == 0)
                return "Temperature data is zero";

            switch (pCounterIds[iCounter]) {
            case 0: // Temperature
                *result = value - 273;
                break;
            case 3: // High Precision Temperature
                *result = value / 10.0 - 273;
                break;
            }
        }

        pCounterData = (PERF_COUNTER_DATA*)((BYTE*)pCounterData + pCounterData->dwSize);
    }

    return NULL;
}

// 7.5
typedef struct FFSmbiosProcessorInfo
{
    FFSmbiosHeader Header;

    uint8_t SocketDesignation; // string
    uint8_t ProcessorType; // enum
    uint8_t ProcessorFamily; // enum
    uint8_t ProcessorManufacturer; // string
    uint64_t ProcessorID; // varies
    uint8_t ProcessorVersion; // string
    uint8_t Voltage; // varies
    uint16_t ExternalClock; // varies
    uint16_t MaxSpeed; // varies
    uint16_t CurrentSpeed; // varies
    uint8_t Status; // varies
    uint8_t ProcessorUpgrade; // enum

    // 2.1+
    uint16_t L1CacheHandle; // varies
    uint16_t L2CacheHandle; // varies
    uint16_t L3CacheHandle; // varies

    // 2.3+
    uint8_t SerialNumber; // string
    uint8_t AssertTag; // string
    uint8_t PartNumber; // string

    // 2.5+
    uint8_t CoreCount; // varies
    uint8_t CoreEnabled; // varies
    uint8_t ThreadCount; // varies
    uint16_t ProcessorCharacteristics; // bit field

    // 2.6+
    uint16_t ProcessorFamily2; // enum

    // 3.0+
    uint16_t CoreCount2; // varies
    uint16_t CoreEnabled2; // varies
    uint16_t ThreadCount2; // varies

    // 3.6+
    uint16_t ThreadEnabled; // varies
} __attribute__((__packed__)) FFSmbiosProcessorInfo;

static_assert(offsetof(FFSmbiosProcessorInfo, ThreadEnabled) == 0x30,
    "FFSmbiosProcessorInfo: Wrong struct alignment");

static const char* detectMaxSpeedBySmbios(FFCPUResult* cpu)
{
    const FFSmbiosHeaderTable* smbiosTable = ffGetSmbiosHeaderTable();
    if (!smbiosTable)
        return "Failed to get SMBIOS data";

    const FFSmbiosProcessorInfo* data = (const FFSmbiosProcessorInfo*) (*smbiosTable)[FF_SMBIOS_TYPE_PROCESSOR_INFO];

    if (!data)
        return "Processor information is not found in SMBIOS data";

    while (data->ProcessorType != 0x03 /*Central Processor*/ || (data->Status & 0b00000111) != 1 /*Enabled*/)
    {
        data = (const FFSmbiosProcessorInfo*) ffSmbiosNextEntry(&data->Header);
        if (data->Header.Type != FF_SMBIOS_TYPE_PROCESSOR_INFO)
            return "No active CPU is found in SMBIOS data";
    }

    uint32_t speed = data->MaxSpeed;
    // Sometimes SMBIOS reports invalid value. We assume that max speed is small than 2x of base
    if (speed < cpu->frequencyBase || speed > cpu->frequencyBase * 2)
        return "Possible invalid CPU max speed in SMBIOS data. See #800";

    cpu->frequencyMax = speed;

    return NULL;
}

static const char* detectNCores(FFCPUResult* cpu)
{
    DWORD length = 0;
    GetLogicalProcessorInformationEx(RelationAll, NULL, &length);
    if (length == 0)
        return "GetLogicalProcessorInformationEx(RelationAll, NULL, &length) failed";

    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* FF_AUTO_FREE
        pProcessorInfo = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(length);

    if (!pProcessorInfo || !GetLogicalProcessorInformationEx(RelationAll, pProcessorInfo, &length))
        return "GetLogicalProcessorInformationEx(RelationAll, pProcessorInfo, &length) failed";

    for(
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* ptr = pProcessorInfo;
        (uint8_t*)ptr < ((uint8_t*)pProcessorInfo) + length;
        ptr = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(((uint8_t*)ptr) + ptr->Size)
    )
    {
        if (ptr->Relationship == RelationGroup)
        {
            for (uint32_t index = 0; index < ptr->Group.ActiveGroupCount; ++index)
            {
                cpu->coresOnline += ptr->Group.GroupInfo[index].ActiveProcessorCount;
                cpu->coresLogical += ptr->Group.GroupInfo[index].MaximumProcessorCount;
            }
        }
        else if (ptr->Relationship == RelationProcessorCore)
            ++cpu->coresPhysical;
        else if (ptr->Relationship == RelationProcessorPackage)
            ++cpu->packages;
    }

    return NULL;
}

static const char* detectByRegistry(FFCPUResult* cpu)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hKey, NULL))
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\", &hKey, NULL) failed";

    ffRegReadStrbuf(hKey, L"ProcessorNameString", &cpu->name, NULL);
    if (ffRegReadStrbuf(hKey, L"VendorIdentifier", &cpu->vendor, NULL))
        ffStrbufTrimRightSpace(&cpu->vendor);

    if (cpu->coresLogical == 0)
    {
        FF_HKEY_AUTO_DESTROY hProcsKey = NULL;
        if (ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor", &hProcsKey, NULL))
        {
            uint32_t cores;
            if (ffRegGetNSubKeys(hProcsKey, &cores, NULL))
                cpu->coresOnline = cpu->coresPhysical = cpu->coresLogical = (uint16_t) cores;
        }
    }

    uint32_t mhz;
    if(ffRegReadUint(hKey, L"~MHz", &mhz, NULL))
        cpu->frequencyBase = mhz;

    return NULL;
}

static const char* detectCoreTypes(FFCPUResult* cpu)
{
    FF_AUTO_FREE PROCESSOR_POWER_INFORMATION* pinfo = calloc(cpu->coresLogical, sizeof(PROCESSOR_POWER_INFORMATION));
    if (!NT_SUCCESS(NtPowerInformation(ProcessorInformation, NULL, 0, pinfo, (ULONG) sizeof(PROCESSOR_POWER_INFORMATION) * cpu->coresLogical)))
        return "NtPowerInformation(ProcessorInformation, NULL, 0, pinfo, size) failed";

    for (uint32_t icore = 0; icore < cpu->coresLogical && pinfo[icore].MhzLimit; ++icore)
    {
        uint32_t ifreq = 0;
        while (cpu->coreTypes[ifreq].freq != pinfo[icore].MhzLimit && cpu->coreTypes[ifreq].freq > 0)
            ++ifreq;
        if (cpu->coreTypes[ifreq].freq == 0)
            cpu->coreTypes[ifreq].freq = pinfo[icore].MhzLimit;
        ++cpu->coreTypes[ifreq].count;
    }

    if (cpu->frequencyBase == 0)
        cpu->frequencyBase = pinfo->MaxMhz;
    return NULL;
}

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    detectNCores(cpu);

    const char* error = detectByRegistry(cpu);
    if (error)
        return error;

    ffCPUDetectByCpuid(cpu);
    if (options->showPeCoreCount) detectCoreTypes(cpu);

    if (cpu->frequencyMax == 0)
        detectMaxSpeedBySmbios(cpu);

    if(options->temp)
        detectThermalTemp(&cpu->temperature);

    return NULL;
}
