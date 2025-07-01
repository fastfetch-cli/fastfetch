#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"

#include "util/mallocHelper.h"

#if !FF_ENABLE_CPUUSAGE_PERFLIB
#include <ntstatus.h>
#include <winternl.h>

const char* ffGetCpuUsageInfo(FFlist* cpuTimes)
{
    ULONG size = 0;
    if(NtQuerySystemInformation(SystemProcessorPerformanceInformation, NULL, 0, &size) != STATUS_INFO_LENGTH_MISMATCH)
        return "NtQuerySystemInformation(SystemProcessorPerformanceInformation, NULL) failed";

    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* FF_AUTO_FREE pinfo = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION*)malloc(size);
    if(!NT_SUCCESS(NtQuerySystemInformation(SystemProcessorPerformanceInformation, pinfo, size, &size)))
        return "NtQuerySystemInformation(SystemProcessorPerformanceInformation, size) failed";

    for (uint32_t i = 0; i < size / sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION); ++i)
    {
        SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION* coreInfo = pinfo + i;

        // KernelTime includes IdleTime and DpcTime.
        coreInfo->KernelTime.QuadPart -= coreInfo->IdleTime.QuadPart;

        uint64_t inUse = (uint64_t) (coreInfo->UserTime.QuadPart + coreInfo->KernelTime.QuadPart);
        uint64_t total = inUse + (uint64_t)coreInfo->IdleTime.QuadPart;

        FFCpuUsageInfo* info = (FFCpuUsageInfo*) ffListAdd(cpuTimes);
        *info = (FFCpuUsageInfo) {
            .inUseAll = inUse,
            .totalAll = total,
        };
    }

    return NULL;
}
#else
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

const char* ffGetCpuUsageInfo(FFlist* cpuTimes)
{
    struct FFPerfQuerySpec
    {
        PERF_COUNTER_IDENTIFIER Identifier;
        WCHAR Name[16];
    } querySpec = {
        .Identifier = {
            // Processor Information GUID
            // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Perflib\_V2Providers\{383487a6-3676-4870-a4e7-d45b30c35629}\{b4fc721a-0378-476f-89ba-a5a79f810b36}
            .CounterSetGuid = { 0xb4fc721a, 0x0378, 0x476f, {0x89, 0xba, 0xa5, 0xa7, 0x9f, 0x81, 0x0b, 0x36} },
            .Size = sizeof(querySpec),
            .CounterId = PERF_WILDCARD_COUNTER, // https://learn.microsoft.com/en-us/windows/win32/perfctrs/using-the-perflib-functions-to-consume-counter-data
            .InstanceId = PERF_WILDCARD_COUNTER,
        },
        .Name = PERF_WILDCARD_INSTANCE,
    };

    __attribute__((__cleanup__(ffPerfCloseQueryHandle)))
        HANDLE hQuery = NULL;

    if (PerfOpenQueryHandle(NULL, &hQuery) != ERROR_SUCCESS)
        return "PerfOpenQueryHandle() failed";

    if (PerfAddCounters(hQuery, &querySpec.Identifier, sizeof(querySpec)) != ERROR_SUCCESS)
        return "PerfAddCounters() failed";

    if (querySpec.Identifier.Status != ERROR_SUCCESS)
        return "PerfAddCounters() reports invalid identifier";

    DWORD dataSize = 0;
    if (PerfQueryCounterData(hQuery, NULL, 0, &dataSize) != ERROR_NOT_ENOUGH_MEMORY)
        return "PerfQueryCounterData(NULL) failed";

    if (dataSize <= sizeof(PERF_DATA_HEADER) + sizeof(PERF_COUNTER_HEADER))
        return "instance doesn't exist";

    FF_AUTO_FREE PERF_DATA_HEADER* const pDataHeader = (PERF_DATA_HEADER*)malloc(dataSize);
    if (PerfQueryCounterData(hQuery, pDataHeader, dataSize, &dataSize) != ERROR_SUCCESS)
        return "PerfQueryCounterData(pDataHeader) failed";

    PERF_COUNTER_HEADER* pCounterHeader = (PERF_COUNTER_HEADER*)(pDataHeader + 1);
    if (pCounterHeader->dwType != PERF_COUNTERSET)
        return "Invalid counter type";

    PERF_MULTI_COUNTERS* pMultiCounters = (PERF_MULTI_COUNTERS*)(pCounterHeader + 1);
    if (pMultiCounters->dwCounters == 0)
        return "No CPU counters found";

    PERF_MULTI_INSTANCES* pMultiInstances = (PERF_MULTI_INSTANCES*)((BYTE*)pMultiCounters + pMultiCounters->dwSize);
    if (pMultiInstances->dwInstances == 0)
        return "No CPU instances found";

    PERF_INSTANCE_HEADER* pInstanceHeader = (PERF_INSTANCE_HEADER*)(pMultiInstances + 1);
    for (DWORD iInstance = 0; iInstance < pMultiInstances->dwInstances; ++iInstance)
    {
        const wchar_t* instanceName = (const wchar_t*)((BYTE*)pInstanceHeader + sizeof(*pInstanceHeader));

        PERF_COUNTER_DATA* pCounterData = (PERF_COUNTER_DATA*)((BYTE*)pInstanceHeader + pInstanceHeader->Size);

        uint64_t processorUtility = UINT64_MAX, utilityBase = UINT64_MAX;
        for (ULONG iCounter = 0; iCounter != pMultiCounters->dwCounters; iCounter++)
        {
            DWORD* pCounterIds = (DWORD*)(pMultiCounters + 1);
            // https://learn.microsoft.com/en-us/windows/win32/perfctrs/using-the-perflib-functions-to-consume-counter-data
            switch (pCounterIds[iCounter]) {
            case 26: // % Processor Utility (#26, Type=PERF_AVERAGE_BULK)
                assert(pCounterData->dwDataSize == sizeof(uint64_t));
                processorUtility = *(uint64_t*)(pCounterData + 1);
                break;
            case 27: // % Utility Base (#27, Type=PERF_AVERAGE_BASE)
                assert(pCounterData->dwDataSize == sizeof(uint32_t));
                utilityBase = *(uint32_t*)(pCounterData + 1) * 100LLU;
                break;
            }

            pCounterData = (PERF_COUNTER_DATA*)((BYTE*)pCounterData + pCounterData->dwSize);
        }

        if (wcschr(instanceName, L'_') == NULL /* ignore `_Total` */)
        {
            if (processorUtility == UINT64_MAX)
                return "Counter \"% Processor Utility\" are not supported";

            FFCpuUsageInfo* info = (FFCpuUsageInfo*) ffListAdd(cpuTimes);
            *info = (FFCpuUsageInfo) {
                .inUseAll = processorUtility,
                .totalAll = utilityBase,
            };
        }

        pInstanceHeader = (PERF_INSTANCE_HEADER*)pCounterData;
    }

    return NULL;
}
#endif
