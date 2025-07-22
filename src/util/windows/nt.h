#pragma once

#include <winnt.h>
#include <winternl.h>

#define D3DKMT_ALIGN64 __attribute__((aligned(8)))

typedef struct _PROCESSOR_POWER_INFORMATION {
  ULONG Number;
  ULONG MaxMhz;
  ULONG CurrentMhz;
  ULONG MhzLimit;
  ULONG MaxIdleState;
  ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

NTSTATUS NTAPI NtPowerInformation(
    IN POWER_INFORMATION_LEVEL InformationLevel,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength);


NTSTATUS NTAPI RtlGetVersion(
    _Inout_ PRTL_OSVERSIONINFOW lpVersionInformation
);

#if __has_include(<d3dkmthk.h>)
#include <d3dkmthk.h>
#else
typedef UINT D3DKMT_HANDLE;

typedef struct _D3DKMT_OPENADAPTERFROMLUID
{
    LUID            AdapterLuid;
    D3DKMT_HANDLE   hAdapter;
} D3DKMT_OPENADAPTERFROMLUID;
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTOpenAdapterFromLuid(_Inout_ CONST D3DKMT_OPENADAPTERFROMLUID*);

typedef struct _D3DKMT_CLOSEADAPTER
{
    D3DKMT_HANDLE   hAdapter;   // in: adapter handle
} D3DKMT_CLOSEADAPTER;
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTCloseAdapter(_In_ CONST D3DKMT_CLOSEADAPTER*);

typedef struct _D3DKMT_ADAPTERTYPE
{
    union
    {
        struct
        {
            UINT   RenderSupported              :  1; // WDDM 1.2, Windows 8
            UINT   DisplaySupported             :  1;
            UINT   SoftwareDevice               :  1;
            UINT   PostDevice                   :  1;
            UINT   HybridDiscrete               :  1; // WDDM 1.3, Windows 8.1
            UINT   HybridIntegrated             :  1;
            UINT   IndirectDisplayDevice        :  1;
            UINT   Paravirtualized              :  1; // WDDM 2.3, Windows 10 Fall Creators Update (version 1709)
            UINT   ACGSupported                 :  1;
            UINT   SupportSetTimingsFromVidPn   :  1;
            UINT   Detachable                   :  1;
            UINT   ComputeOnly                  :  1; // WDDM 2.6, Windows 10 May 2019 Update (Version 1903)
            UINT   Prototype                    :  1;
            UINT   RuntimePowerManagement       :  1; // WDDM 2.9, Windows 10 Insider Preview "Iron"
            UINT   Reserved                     : 18;
        };
        UINT Value;
    };
} D3DKMT_ADAPTERTYPE;

typedef enum _KMTQUERYADAPTERINFOTYPE
{
    KMTQAITYPE_ADAPTERTYPE = 15,  // WDDM 1.2, Windows 8
    KMTQAITYPE_NODEMETADATA = 25, // WDDM 2.0, Windows 10
} KMTQUERYADAPTERINFOTYPE;
typedef struct _D3DKMT_QUERYADAPTERINFO
{
    D3DKMT_HANDLE           hAdapter;
    KMTQUERYADAPTERINFOTYPE Type;
    VOID*                   pPrivateDriverData;
    UINT                    PrivateDriverDataSize;
} D3DKMT_QUERYADAPTERINFO;
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTQueryAdapterInfo(_Inout_ CONST D3DKMT_QUERYADAPTERINFO*);

typedef enum _D3DKMT_QUERYSTATISTICS_TYPE
{
    D3DKMT_QUERYSTATISTICS_PHYSICAL_ADAPTER       = 10, // WDDM 2.4, Windows 10 April 2018 Update (version 1803)
    D3DKMT_QUERYSTATISTICS_NODE2                  = 18, // WDDM 3.1, Windows 11 2022 Update (version 22H2)
} D3DKMT_QUERYSTATISTICS_TYPE;
typedef struct _D3DKMT_QUERYSTATISTICS_QUERY_PHYSICAL_ADAPTER
{
    ULONG PhysicalAdapterIndex;
} D3DKMT_QUERYSTATISTICS_QUERY_PHYSICAL_ADAPTER;
typedef struct _D3DKMT_QUERYSTATISTICS_QUERY_NODE2
{
    UINT16 PhysicalAdapterIndex;
    UINT16 NodeOrdinal;
} D3DKMT_QUERYSTATISTICS_QUERY_NODE2;
typedef struct _D3DKMT_ADAPTER_PERFDATA
{
    UINT32          PhysicalAdapterIndex;   // in: The physical adapter index, in an LDA chain
    D3DKMT_ALIGN64 ULONGLONG MemoryFrequency;        // out: Clock frequency of the memory in hertz
    D3DKMT_ALIGN64 ULONGLONG MaxMemoryFrequency;     // out: Max memory clock frequency
    D3DKMT_ALIGN64 ULONGLONG MaxMemoryFrequencyOC;   // out: Clock frequency of the memory while overclocked in hertz.
    D3DKMT_ALIGN64 ULONGLONG MemoryBandwidth;        // out: Amount of memory transferred in bytes
    D3DKMT_ALIGN64 ULONGLONG PCIEBandwidth;          // out: Amount of memory transferred over PCI-E in bytes
    ULONG           FanRPM;                 // out: Fan rpm
    ULONG           Power;                  // out: Power draw of the adapter in tenths of a percentage
    ULONG           Temperature;            // out: Temperature in deci-Celsius 1 = 0.1C
    UCHAR           PowerStateOverride;     // out: Overrides dxgkrnls power view of linked adapters.
} D3DKMT_ADAPTER_PERFDATA;
typedef struct _D3DKMT_ADAPTER_PERFDATACAPS
{
    UINT32      PhysicalAdapterIndex;   // in: The physical adapter index, in an LDA chain
    D3DKMT_ALIGN64 ULONGLONG MaxMemoryBandwidth;     // out: Max memory bandwidth in bytes for 1 second
    D3DKMT_ALIGN64 ULONGLONG MaxPCIEBandwidth;       // out: Max pcie bandwidth in bytes for 1 second
    ULONG       MaxFanRPM;              // out: Max fan rpm
    ULONG       TemperatureMax;         // out: Max temperature before damage levels
    ULONG       TemperatureWarning;     // out: The temperature level where throttling begins.
} D3DKMT_ADAPTER_PERFDATACAPS;

#define DXGK_MAX_GPUVERSION_NAME_LENGTH 32
typedef struct _D3DKMT_GPUVERSION
{
    UINT32          PhysicalAdapterIndex;                             // in: The physical adapter index, in an LDA chain
    WCHAR           BiosVersion[DXGK_MAX_GPUVERSION_NAME_LENGTH];     //out: The gpu bios version
    WCHAR           GpuArchitecture[DXGK_MAX_GPUVERSION_NAME_LENGTH]; //out: The gpu architectures name.
} D3DKMT_GPUVERSION;
typedef struct _D3DKMT_QUERYSTATISTICS_PHYSICAL_ADAPTER_INFORMATION
{
    D3DKMT_ADAPTER_PERFDATA      AdapterPerfData;
    D3DKMT_ADAPTER_PERFDATACAPS  AdapterPerfDataCaps;
    D3DKMT_GPUVERSION            GpuVersion;
} D3DKMT_QUERYSTATISTICS_PHYSICAL_ADAPTER_INFORMATION;
typedef struct _D3DKMT_QUERYSTATISTICS_PROCESS_NODE_INFORMATION {
    D3DKMT_ALIGN64 UINT64                         Reserved[34];
} D3DKMT_QUERYSTATISTICS_PROCESS_NODE_INFORMATION;
typedef struct _D3DKMT_NODE_PERFDATA
{
    UINT32          NodeOrdinal;            // in: Node ordinal of the requested engine.
    UINT32          PhysicalAdapterIndex;   // in: The physical adapter index, in an LDA chain
    D3DKMT_ALIGN64 ULONGLONG Frequency;     // out: Clock frequency of the engine in hertz
    D3DKMT_ALIGN64 ULONGLONG MaxFrequency;  // out: Max engine clock frequency
    D3DKMT_ALIGN64 ULONGLONG MaxFrequencyOC;// out: Max engine over clock frequency
    ULONG           Voltage;                // out: Voltage of the engine in milli volts mV
    ULONG           VoltageMax;             // out: Max voltage levels in milli volts.
    ULONG           VoltageMaxOC;           // out: Max voltage level while overclocked in milli volts.
    // WDDM 2.5
    D3DKMT_ALIGN64 ULONGLONG MaxTransitionLatency;   // out: Max transition latency to change the frequency in 100 nanoseconds
} D3DKMT_NODE_PERFDATA;
typedef struct _D3DKMT_QUERYSTATISTICS_NODE_INFORMATION {
    D3DKMT_QUERYSTATISTICS_PROCESS_NODE_INFORMATION GlobalInformation; //Global statistics
    D3DKMT_QUERYSTATISTICS_PROCESS_NODE_INFORMATION SystemInformation; //Statistics for system thread
    D3DKMT_NODE_PERFDATA                            NodePerfData;
    UINT32                                          Reserved[3];
} D3DKMT_QUERYSTATISTICS_NODE_INFORMATION;
typedef union _D3DKMT_QUERYSTATISTICS_RESULT
{
    D3DKMT_QUERYSTATISTICS_PHYSICAL_ADAPTER_INFORMATION PhysAdapterInformation;
    D3DKMT_QUERYSTATISTICS_NODE_INFORMATION NodeInformation;
    uint8_t Padding[776];
} D3DKMT_QUERYSTATISTICS_RESULT;
typedef struct _D3DKMT_QUERYSTATISTICS
{
    D3DKMT_QUERYSTATISTICS_TYPE   Type;        // in: type of data requested
    LUID                          AdapterLuid; // in: adapter to get export / statistics from
    HANDLE*                       hProcess;    // in: process to get statistics for, if required for this query type
    D3DKMT_QUERYSTATISTICS_RESULT QueryResult; // out: requested data

    union
    {
        D3DKMT_QUERYSTATISTICS_QUERY_PHYSICAL_ADAPTER QueryPhysAdapter; // in: id of physical adapter to get statistics for
        D3DKMT_QUERYSTATISTICS_QUERY_NODE2 QueryNode2; // in: id of node to get statistics for
    };
} D3DKMT_QUERYSTATISTICS;
static_assert(sizeof(D3DKMT_QUERYSTATISTICS) ==
    #if _WIN64
    0x328
    #else
    0x320
    #endif
, "D3DKMT_QUERYSTATISTICS structure size mismatch");
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTQueryStatistics(_In_ CONST D3DKMT_QUERYSTATISTICS*);

#define DXGK_MAX_METADATA_NAME_LENGTH 32
typedef enum
{
    DXGK_ENGINE_TYPE_OTHER,
    DXGK_ENGINE_TYPE_3D,
    DXGK_ENGINE_TYPE_VIDEO_DECODE,
    DXGK_ENGINE_TYPE_VIDEO_ENCODE,
    DXGK_ENGINE_TYPE_VIDEO_PROCESSING,
    DXGK_ENGINE_TYPE_SCENE_ASSEMBLY,
    DXGK_ENGINE_TYPE_COPY,
    DXGK_ENGINE_TYPE_OVERLAY,
    DXGK_ENGINE_TYPE_CRYPTO,
    DXGK_ENGINE_TYPE_VIDEO_CODEC,
    DXGK_ENGINE_TYPE_MAX
} DXGK_ENGINE_TYPE;
typedef struct _DXGK_NODEMETADATA_FLAGS
{
    union
    {
        struct
        {
            UINT ContextSchedulingSupported :  1; // WDDM 2.2
            UINT RingBufferFenceRelease     :  1; // WDDM 2.5
            UINT SupportTrackedWorkload     :  1;
            UINT UserModeSubmission         :  1;
            UINT SupportBuildTestCommandBuffer :  1; // WDDM 3.2
            UINT Reserved                   : 11;
            UINT MaxInFlightHwQueueBuffers  : 16;
        };
        UINT32 Value;
    };
} DXGK_NODEMETADATA_FLAGS;
typedef struct _DXGK_NODEMETADATA
{
    DXGK_ENGINE_TYPE EngineType;
    WCHAR            FriendlyName[DXGK_MAX_METADATA_NAME_LENGTH];
    DXGK_NODEMETADATA_FLAGS Flags; // WDDM 2.2
    BOOLEAN          GpuMmuSupported; // WDDM 2.0 ???
    BOOLEAN          IoMmuSupported;
} __attribute__((packed))  DXGK_NODEMETADATA;
typedef struct _D3DKMT_NODEMETADATA
{
    _In_ UINT NodeOrdinalAndAdapterIndex;     // WDDMv2: High word is physical adapter index, low word is node ordinal
    _Out_ DXGK_NODEMETADATA NodeData;
} __attribute__((packed))  D3DKMT_NODEMETADATA;
static_assert(sizeof(D3DKMT_NODEMETADATA) == 0x4E, "D3DKMT_NODEMETADATA structure size mismatch");

#endif
