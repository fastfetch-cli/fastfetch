#include "fastfetch.h"
#include "detection/cpuusage/cpuusage.h"

#include "util/mallocHelper.h"

#include <ntstatus.h>
#include <winternl.h>
#include <windows.h>
#include <wchar.h>
#include "util/windows/perflib_.h"

static const char* getInfoByNqsi(FFlist* cpuTimes)
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

static const char* getInfoByPerflib(FFlist* cpuTimes)
{
    static HANDLE hQuery = NULL;

    if (hQuery == NULL)
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

        if (PerfOpenQueryHandle(NULL, &hQuery) != ERROR_SUCCESS)
        {
            PerfCloseQueryHandle(hQuery);
            hQuery = INVALID_HANDLE_VALUE;
            return "PerfOpenQueryHandle() failed";
        }

        if (PerfAddCounters(hQuery, &querySpec.Identifier, sizeof(querySpec)) != ERROR_SUCCESS)
        {
            PerfCloseQueryHandle(hQuery);
            hQuery = INVALID_HANDLE_VALUE;
            return "PerfAddCounters() failed";
        }

        if (querySpec.Identifier.Status != ERROR_SUCCESS)
        {
            PerfCloseQueryHandle(hQuery);
            hQuery = INVALID_HANDLE_VALUE;
            return "PerfAddCounters() reports invalid identifier";
        }
    }

    if (hQuery == INVALID_HANDLE_VALUE)
        return "Init hQuery failed";

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

const char* ffGetCpuUsageInfo(FFlist* cpuTimes)
{
    #if __aarch64__
    static uint8_t winver = 10; // Assume Windows 10 or later for WoA
    #else
    static uint8_t winver = 0;
    if (winver == 0)
        winver = (uint8_t) ffStrbufToUInt(&instance.state.platform.sysinfo.release, 1);
    #endif

    if (winver >= 10)
    {
        if (getInfoByPerflib(cpuTimes) == NULL) return NULL;
        ffListClear(cpuTimes);
        winver = 1; // Fall back to NQSI
    }

    return getInfoByNqsi(cpuTimes);
}
