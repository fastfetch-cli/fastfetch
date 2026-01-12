#pragma once

#include <windows.h>
#include <perflib.h>

// Missing from <perflib.h> of MinGW-w64 SDK

#define PERF_WILDCARD_COUNTER   0xFFFFFFFF
#define PERF_WILDCARD_INSTANCE  L"*"
#define PERF_AGGREGATE_INSTANCE L"_Total"
#define PERF_MAX_INSTANCE_NAME  1024

typedef struct _PERF_INSTANCE_HEADER {
    ULONG Size;       // = sizeof(PERF_INSTANCE_HEADER) + sizeof(InstanceName) + sizeof(Padding)
    ULONG InstanceId; // Instance ID.
    // Followed by:
    // WCHAR InstanceName[]; // Nul-terminated.
    // WCHAR Padding[];      // Pad to a multiple of 8 bytes
} PERF_INSTANCE_HEADER, *PPERF_INSTANCE_HEADER;

typedef struct _PERF_COUNTER_IDENTIFIER {
    GUID   CounterSetGuid; // The GUID of the counterset.
    ULONG  Status;         // Win32 error code indicating success/failure of the add/delete operation.
    ULONG  Size;           // sizeof(PERF_COUNTER_IDENTIFIER) + sizeof(InstanceName) + sizeof(Padding)
    ULONG  CounterId;      // CounterId, or PERF_WILDCARD_COUNTER for all counters.
    ULONG  InstanceId;     // InstanceId, or 0xFFFFFFFF to not filter on instance ID.
    ULONG  Index;          // Set by PerfQueryCounterInfo to the position in which the corresponding counter data is returned.
    ULONG  Reserved;       // Reserved.
    // Followed by:
    // WCHAR InstanceName[];
    // WCHAR Padding[];
} PERF_COUNTER_IDENTIFIER, * PPERF_COUNTER_IDENTIFIER;

typedef struct _PERF_DATA_HEADER {
    ULONG      dwTotalSize;     // = sizeof(PERF_DATA_HEADER) + sizeof(PERF_COUNTER_HEADER blocks...)
    ULONG      dwNumCounters;   // The number of PERF_COUNTER_HEADER blocks.
    LONGLONG   PerfTimeStamp;   // Timestamp from a high-resolution clock.
    LONGLONG   PerfTime100NSec; // The number of 100 nanosecond intervals since January 1, 1601, in Coordinated Universal Time (UTC).
    LONGLONG   PerfFreq;        // The frequency of a high-resolution clock.
    SYSTEMTIME SystemTime;      // The time at which data is collected on the provider side.
    // Followed by:
    // PERF_COUNTER_HEADER blocks...;
} PERF_DATA_HEADER, * PPERF_DATA_HEADER;

typedef enum _PerfCounterDataType {
    PERF_ERROR_RETURN = 0,       /* An error occurred when the performance counter value was queried. */
    PERF_SINGLE_COUNTER = 1,     /* Query returned a single counter from a single-instance. */
    PERF_MULTIPLE_COUNTERS = 2,  /* Query returned multiple counters from a single instance. */
    PERF_MULTIPLE_INSTANCES = 4, /* Query returned a single counter from each of multiple instances. */
    PERF_COUNTERSET = 6          /* Query returned multiple counters from each of multiple instances. */
} PerfCounterDataType;

typedef struct _PERF_COUNTER_HEADER {
    ULONG      dwStatus;        // Win32 error code indicating success/failure of the query operation.
    PerfCounterDataType dwType; // Result type - error, single/single, multi/single, single/multi, multi/multi.
    ULONG      dwSize;          // = sizeof(PERF_COUNTER_HEADER) + sizeof(Additional data)
    ULONG      Reserved;        // Reserved.
    // Followed by additional data:
    // If dwType == PERF_ERROR_RETURN:       nothing.
    // If dwType == PERF_SINGLE_COUNTER:     PERF_COUNTER_DATA block.
    // If dwType == PERF_MULTIPLE_COUNTERS:  PERF_MULTI_COUNTERS block + PERF_COUNTER_DATA blocks.
    // If dwType == PERF_MULTIPLE_INSTANCES: PERF_MULTI_INSTANCES block.
    // If dwType == PERF_COUNTERSET:         PERF_MULTI_COUNTERS block + PERF_MULTI_INSTANCES block.
} PERF_COUNTER_HEADER, * PPERF_COUNTER_HEADER;

typedef struct _PERF_MULTI_INSTANCES {
    ULONG      dwTotalSize; // = sizeof(PERF_MULTI_INSTANCES) + sizeof(instance data blocks...)
    ULONG      dwInstances; // Number of instance data blocks.
    // Followed by:
    // Instance data blocks...;
} PERF_MULTI_INSTANCES, * PPERF_MULTI_INSTANCES;

typedef struct _PERF_MULTI_COUNTERS {
    ULONG      dwSize;     // sizeof(PERF_MULTI_COUNTERS) + sizeof(CounterIds)
    ULONG      dwCounters; // Number of counter ids.
    // Followed by:
    // DWORD CounterIds[dwCounters];
} PERF_MULTI_COUNTERS, * PPERF_MULTI_COUNTERS;

typedef struct _PERF_COUNTER_DATA {
    ULONG      dwDataSize; // Size of the counter data, in bytes.
    ULONG      dwSize;     // = sizeof(PERF_COUNTER_DATA) + sizeof(Data) + sizeof(Padding)
    // Followed by:
    // BYTE Data[dwDataSize];
    // BYTE Padding[];
} PERF_COUNTER_DATA, * PPERF_COUNTER_DATA;

_Success_(return == ERROR_SUCCESS)
ULONG
WINAPI
PerfEnumerateCounterSetInstances(
    _In_opt_z_ LPCWSTR szMachine,
    _In_ LPCGUID pCounterSetId,
    _Out_opt_bytecap_post_bytecount_(cbInstances, *pcbInstancesActual) PPERF_INSTANCE_HEADER pInstances,
    DWORD cbInstances,
    _Out_ LPDWORD pcbInstancesActual
    );

_Success_(return == ERROR_SUCCESS)
ULONG
WINAPI
PerfOpenQueryHandle(
    _In_opt_z_ LPCWSTR szMachine,
    _Out_ HANDLE * phQuery
    );

_Success_(return == ERROR_SUCCESS)
ULONG
WINAPI
PerfCloseQueryHandle(
    _In_ HANDLE hQuery
    );

_Success_(return == ERROR_SUCCESS)
ULONG
WINAPI
PerfAddCounters(
    _In_ HANDLE hQuery,
    _Inout_bytecount_(cbCounters) PPERF_COUNTER_IDENTIFIER pCounters,
    DWORD cbCounters
    );

_Success_(return == ERROR_SUCCESS)
ULONG
WINAPI
PerfDeleteCounters(
    _In_ HANDLE hQuery,
    _Inout_bytecount_(cbCounters) PPERF_COUNTER_IDENTIFIER pCounters,
    DWORD cbCounters
    );

_Success_(return == ERROR_SUCCESS)
ULONG
WINAPI
PerfQueryCounterData(
    _In_ HANDLE hQuery,
    _Out_opt_bytecap_post_bytecount_(cbCounterBlock, *pcbCounterBlockActual) PPERF_DATA_HEADER pCounterBlock,
    DWORD cbCounterBlock,
    _Out_ LPDWORD pcbCounterBlockActual
    );
