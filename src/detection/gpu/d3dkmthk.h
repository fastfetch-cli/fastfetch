#pragma once

#include <stdint.h>
#include <limits.h>
#include <assert.h>

#if _WIN32
#include <ntdef.h>
#include <windef.h>
#else
#include <sys/ioctl.h>
#include <uchar.h>

typedef struct _LUID {
    uint32_t LowPart;
    uint32_t HighPart;
} LUID;

typedef uint32_t UINT;
typedef uint64_t ULONGLONG;
typedef uint64_t UINT64;
typedef uint32_t UINT32;
typedef uint16_t UINT16;
typedef uint32_t ULONG;
typedef uint16_t USHORT;
typedef uint8_t UCHAR;
typedef void VOID;
typedef char16_t WCHAR;
typedef void* HANDLE;
typedef uint8_t BYTE;
typedef int32_t BOOL;
typedef uint8_t BOOLEAN;
typedef union {
    struct {
        uint32_t LowPart;
        int32_t HighPart;
    } u;
    int64_t QuadPart;
} LARGE_INTEGER;
typedef int32_t NTSTATUS; // 0 for success, -1 for failure
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif

#define D3DKMT_ALIGN64 __attribute__((aligned(8)))
#define MAX_ENUM_ADAPTERS   16

typedef struct D3DKMT_HANDLE
{
    union {
        struct {
            uint32_t Instance :  6;
            uint32_t Index    : 24;
            uint32_t Unique   :  2;
        };
        uint32_t Value;
    };
} D3DKMT_HANDLE;

typedef struct _D3DKMT_OPENADAPTERFROMLUID
{
    LUID            AdapterLuid;
    D3DKMT_HANDLE   hAdapter;
} D3DKMT_OPENADAPTERFROMLUID;

typedef struct _D3DKMT_ADAPTERINFO
{
    D3DKMT_HANDLE       hAdapter;
    LUID                AdapterLuid;
    ULONG               NumOfSources;
    BOOL                bPrecisePresentRegionsPreferred;
} D3DKMT_ADAPTERINFO;

typedef struct _D3DKMT_ENUMADAPTERS2
{
    ULONG                 NumAdapters;           // in/out: On input, the count of the pAdapters array buffer.  On output, the number of adapters enumerated.
    D3DKMT_ADAPTERINFO*   pAdapters;             // out: Array of enumerated adapters containing NumAdapters elements
} D3DKMT_ENUMADAPTERS2;
static_assert(sizeof(D3DKMT_ENUMADAPTERS2) ==
    #if SIZE_MAX == UINT64_MAX
    0x10
    #else
    0x08
    #endif
, "D3DKMT_ENUMADAPTERS2 structure size mismatch");

typedef struct _D3DKMT_ADAPTERREGISTRYINFO
{
    WCHAR   AdapterString[260];
    WCHAR   BiosString[260];
    WCHAR   DacType[260];
    WCHAR   ChipType[260];
} D3DKMT_ADAPTERREGISTRYINFO;

typedef struct _D3DKMT_CLOSEADAPTER
{
    D3DKMT_HANDLE   hAdapter;   // in: adapter handle
} D3DKMT_CLOSEADAPTER;

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

typedef struct _D3DKMT_ADAPTERADDRESS
{
    UINT   BusNumber;              // Bus number on which the physical device is located.
    UINT   DeviceNumber;           // Index of the physical device on the bus.
    UINT   FunctionNumber;         // Function number of the adapter on the physical device.
} D3DKMT_ADAPTERADDRESS;

typedef struct _D3DKMT_DEVICE_IDS
{
    UINT VendorID;
    UINT DeviceID;
    UINT SubVendorID;
    UINT SubSystemID;
    UINT RevisionID;
    UINT BusType;
} D3DKMT_DEVICE_IDS;

typedef struct _D3DKMT_UMD_DRIVER_VERSION
{
    D3DKMT_ALIGN64 LARGE_INTEGER DriverVersion;
} D3DKMT_UMD_DRIVER_VERSION;

typedef struct _D3DKMT_QUERY_DEVICE_IDS
{
    UINT              PhysicalAdapterIndex; // in:
    D3DKMT_DEVICE_IDS DeviceIds;            // out:
} D3DKMT_QUERY_DEVICE_IDS;

typedef enum _QAI_DRIVERVERSION
{
    KMT_DRIVERVERSION_WDDM_1_0 = 1000, // Windows Vista
    KMT_DRIVERVERSION_WDDM_1_1_PRERELEASE = 1102, // Windows Vista with prereleased Win7 features
    KMT_DRIVERVERSION_WDDM_1_1 = 1105, // Windows 7
    KMT_DRIVERVERSION_WDDM_1_2 = 1200, // Windows 8
    KMT_DRIVERVERSION_WDDM_1_3 = 1300, // Windows 8.1
    KMT_DRIVERVERSION_WDDM_2_0 = 2000, // Windows 10
    KMT_DRIVERVERSION_WDDM_2_1 = 2100, // Windows 10 (1607)
    KMT_DRIVERVERSION_WDDM_2_2 = 2200, // Windows 10 (1703)
    KMT_DRIVERVERSION_WDDM_2_3 = 2300, // Windows 10 (1709)
    KMT_DRIVERVERSION_WDDM_2_4 = 2400, // Windows 10 (1803)
    KMT_DRIVERVERSION_WDDM_2_5 = 2500, // Windows 10 (1809)
    KMT_DRIVERVERSION_WDDM_2_6 = 2600, // Windows 10 (1903)
    KMT_DRIVERVERSION_WDDM_2_7 = 2700, // Windows 10 (2004)
    KMT_DRIVERVERSION_WDDM_2_8 = 2800, // Windows 11 Insider Preview Manganese
    KMT_DRIVERVERSION_WDDM_2_9 = 2900, // Windows 11 Insider Preview Iron
    KMT_DRIVERVERSION_WDDM_3_0 = 3000, // Windows 11 (21H2)
    KMT_DRIVERVERSION_WDDM_3_1 = 3100, // Windows 11 (22H2)
    KMT_DRIVERVERSION_WDDM_3_2 = 3200, // Windows 11 (24H2)
} D3DKMT_DRIVERVERSION;

typedef struct _D3DKMT_QUERY_ADAPTER_UNIQUE_GUID
{
    WCHAR AdapterUniqueGUID[40];
} D3DKMT_QUERY_ADAPTER_UNIQUE_GUID;

typedef enum _KMTQUERYADAPTERINFOTYPE
{
    KMTQAITYPE_ADAPTERGUID               = 4,
    KMTQAITYPE_ADAPTERADDRESS            = 6,
    KMTQAITYPE_ADAPTERREGISTRYINFO       = 8,
    KMTQAITYPE_DRIVERVERSION             = 13,
    KMTQAITYPE_ADAPTERTYPE               = 15,  // WDDM 1.2, Windows 8
    KMTQAITYPE_UMD_DRIVER_VERSION        = 18,
    KMTQAITYPE_NODEMETADATA              = 25,  // WDDM 2.0, Windows 10
    KMTQAITYPE_PHYSICALADAPTERDEVICEIDS  = 31,
    KMTQAITYPE_QUERY_ADAPTER_UNIQUE_GUID = 60,  // WDDM 2.4, Windows 10 (1803)
                                                // This reports the GUID string used by the adapter's registry key (DirectX and Video)
} KMTQUERYADAPTERINFOTYPE;

typedef struct _D3DKMT_QUERYADAPTERINFO
{
    D3DKMT_HANDLE           hAdapter;
    KMTQUERYADAPTERINFOTYPE Type;
    VOID*                   pPrivateDriverData;
    UINT                    PrivateDriverDataSize;
} D3DKMT_QUERYADAPTERINFO;

#define D3DKMT_MAX_SEGMENT_COUNT 32

typedef enum _D3DKMT_MEMORY_SEGMENT_GROUP
{
    D3DKMT_MEMORY_SEGMENT_GROUP_LOCAL = 0,
    D3DKMT_MEMORY_SEGMENT_GROUP_NON_LOCAL = 1
} D3DKMT_MEMORY_SEGMENT_GROUP;

typedef enum _D3DKMT_QUERYSTATISTICS_TYPE
{
    D3DKMT_QUERYSTATISTICS_PHYSICAL_ADAPTER       = 10, // WDDM 2.4, Windows 10 April 2018 Update (version 1803)
    D3DKMT_QUERYSTATISTICS_SEGMENT_GROUP_USAGE    = 17, // WDDM 3.1, Windows 11 2022 Update (version 22H2)
    D3DKMT_QUERYSTATISTICS_NODE2                  = 18,
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

typedef struct _D3DKMT_QUERYSTATISTICS_MEMORY_USAGE
{
    D3DKMT_ALIGN64 UINT64 AllocatedBytes;
    D3DKMT_ALIGN64 UINT64 FreeBytes;
    D3DKMT_ALIGN64 UINT64 ZeroBytes;
    D3DKMT_ALIGN64 UINT64 ModifiedBytes;
    D3DKMT_ALIGN64 UINT64 StandbyBytes;
} D3DKMT_QUERYSTATISTICS_MEMORY_USAGE;

typedef union _D3DKMT_QUERYSTATISTICS_RESULT
{
    D3DKMT_QUERYSTATISTICS_PHYSICAL_ADAPTER_INFORMATION PhysAdapterInformation;
    D3DKMT_QUERYSTATISTICS_NODE_INFORMATION NodeInformation;
    D3DKMT_QUERYSTATISTICS_MEMORY_USAGE SegmentGroupUsageInformation;
    uint8_t Padding[776];
} D3DKMT_QUERYSTATISTICS_RESULT;

typedef struct _D3DKMT_QUERYSTATISTICS_QUERY_SEGMENT_GROUP_USAGE
{
    UINT16 PhysicalAdapterIndex;
    UINT16 SegmentGroup; // D3DKMT_MEMORY_SEGMENT_GROUP
} D3DKMT_QUERYSTATISTICS_QUERY_SEGMENT_GROUP_USAGE;

typedef struct _D3DKMT_QUERYSTATISTICS
{
    D3DKMT_QUERYSTATISTICS_TYPE   Type;        // in: type of data requested
    LUID                          AdapterLuid; // in: adapter to get export / statistics from
    HANDLE*                       hProcess;    // in: process to get statistics for, if required for this query type
    D3DKMT_QUERYSTATISTICS_RESULT QueryResult; // out: requested data

    union
    {
        D3DKMT_QUERYSTATISTICS_QUERY_PHYSICAL_ADAPTER QueryPhysAdapter; // in: id of physical adapter to get statistics for
        D3DKMT_QUERYSTATISTICS_QUERY_SEGMENT_GROUP_USAGE QuerySegmentGroupUsage;
        D3DKMT_QUERYSTATISTICS_QUERY_NODE2 QueryNode2; // in: id of node to get statistics for
    };
} D3DKMT_QUERYSTATISTICS;
static_assert(sizeof(D3DKMT_QUERYSTATISTICS) ==
    #if SIZE_MAX == UINT64_MAX
    0x328
    #else
    0x320
    #endif
, "D3DKMT_QUERYSTATISTICS structure size mismatch");

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
    UINT NodeOrdinalAndAdapterIndex;     // WDDMv2: High word is physical adapter index, low word is node ordinal
    DXGK_NODEMETADATA NodeData;
} __attribute__((packed))  D3DKMT_NODEMETADATA;
static_assert(sizeof(D3DKMT_NODEMETADATA) == 0x4E, "D3DKMT_NODEMETADATA structure size mismatch");

// Functions

#if _WIN32

EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTOpenAdapterFromLuid(_Inout_ CONST D3DKMT_OPENADAPTERFROMLUID*);
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTQueryAdapterInfo(_Inout_ CONST D3DKMT_QUERYADAPTERINFO*);
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTCloseAdapter(_In_ CONST D3DKMT_CLOSEADAPTER*);
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTEnumAdapters2(_Inout_ D3DKMT_ENUMADAPTERS2*);
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTQueryStatistics(_In_ CONST D3DKMT_QUERYSTATISTICS*);

#else

// Ref: https://github.com/microsoft/WSL2-Linux-Kernel/blob/linux-msft-wsl-6.6.y/include/uapi/misc/d3dkmthk.h
#define LX_DXOPENADAPTERFROMLUID    _IOWR(0x47, 0x01, D3DKMT_OPENADAPTERFROMLUID)
#define LX_DXQUERYADAPTERINFO       _IOWR(0x47, 0x09, D3DKMT_QUERYADAPTERINFO)
#define LX_DXENUMADAPTERS2          _IOWR(0x47, 0x14, D3DKMT_ENUMADAPTERS2)
#define LX_DXCLOSEADAPTER           _IOWR(0x47, 0x15, D3DKMT_CLOSEADAPTER)
#define LX_DXQUERYSTATISTICS        _IOWR(0x47, 0x43, D3DKMT_QUERYSTATISTICS)

static inline NTSTATUS D3DKMTOpenAdapterFromLuid(int dxgfd, const D3DKMT_OPENADAPTERFROMLUID* params)
{
    return ioctl(dxgfd, LX_DXOPENADAPTERFROMLUID, params);
}

static inline NTSTATUS D3DKMTQueryAdapterInfo(int dxgfd, const D3DKMT_QUERYADAPTERINFO* params)
{
    return ioctl(dxgfd, LX_DXQUERYADAPTERINFO, params);
}

static inline NTSTATUS D3DKMTCloseAdapter(int dxgfd, const D3DKMT_CLOSEADAPTER* params)
{
    return ioctl(dxgfd, LX_DXCLOSEADAPTER, params);
}

static inline NTSTATUS D3DKMTEnumAdapters2(int dxgfd, D3DKMT_ENUMADAPTERS2* params)
{
    return ioctl(dxgfd, LX_DXENUMADAPTERS2, params);
}

static inline NTSTATUS D3DKMTQueryStatistics(int dxgfd, const D3DKMT_QUERYSTATISTICS* params)
{
    return ioctl(dxgfd, LX_DXQUERYSTATISTICS, params);
}

#endif
