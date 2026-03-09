#pragma once

#include <ntdef.h>
#include <winternl.h>
#include <winnt.h>
#include <stdint.h>
#include <assert.h>

enum {
    SystemModuleInformation = 11,
    SystemFirmwareTableInformation = 76,
    SystemBootEnvironmentInformation = 90,
    SystemLogicalProcessorAndGroupInformation = 107,
    SystemSecureBootInformation = 146,
};

#define D3DKMT_ALIGN64 __attribute__((aligned(8)))

typedef struct _PROCESSOR_POWER_INFORMATION {
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

NTSYSAPI NTSTATUS NTAPI NtPowerInformation(
    IN POWER_INFORMATION_LEVEL InformationLevel,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength);


NTSYSAPI NTSTATUS NTAPI RtlGetVersion(
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

NTSYSAPI NTSTATUS NTAPI NtQueryDirectoryFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN BOOLEAN RestartScan);

// https://ntdoc.m417z.com/process_devicemap_information_ex
typedef struct _PROCESS_DEVICEMAP_INFORMATION_EX
{
    union
    {
        struct
        {
            HANDLE DirectoryHandle; // A handle to a directory object that can be set as the new device map for the process. This handle must have DIRECTORY_TRAVERSE access.
        } Set;
        struct
        {
            ULONG DriveMap;         // A bitmask that indicates which drive letters are currently in use in the process's device map.
            UCHAR DriveType[32];    // A value that indicates the type of each drive (e.g., local disk, network drive, etc.). // DRIVE_* WinBase.h
        } Query;
    };
    ULONG Flags; // PROCESS_LUID_DOSDEVICES_ONLY
} PROCESS_DEVICEMAP_INFORMATION_EX, *PPROCESS_DEVICEMAP_INFORMATION_EX;

#ifndef NtCurrentProcess
#define NtCurrentProcess() ((HANDLE)(LONG_PTR)-1)
#endif

typedef struct _CURDIR
{
    UNICODE_STRING DosPath;
    HANDLE Handle;
} CURDIR, *PCURDIR;

NTSYSAPI PIMAGE_NT_HEADERS NTAPI RtlImageNtHeader(IN PVOID BaseOfImage);

/**
 * The SECTION_IMAGE_INFORMATION structure contains detailed information about an image section.
 */
typedef struct _SECTION_IMAGE_INFORMATION
{
    PVOID TransferAddress;          // The address of the image entry point function.
    ULONG ZeroBits;                 // The number of high-order address bits that must be zero in the image base address.
    SIZE_T MaximumStackSize;        // The maximum stack size of threads from the PE file header.
    SIZE_T CommittedStackSize;      // The initial stack size of threads from the PE file header.
    ULONG SubSystemType;            // The image subsystem from the PE file header (e.g., Windows GUI, Windows CUI, POSIX).
    union
    {
        struct
        {
            USHORT SubSystemMinorVersion;
            USHORT SubSystemMajorVersion;
        };
        ULONG SubSystemVersion;
    };
    union
    {
        struct
        {
            USHORT MajorOperatingSystemVersion;
            USHORT MinorOperatingSystemVersion;
        };
        ULONG OperatingSystemVersion;
    };
    USHORT ImageCharacteristics;    // The image characteristics from the PE file header.
    USHORT DllCharacteristics;      // The DLL characteristics flags (e.g., ASLR, NX compatibility).
    USHORT Machine;                 // The image architecture (e.g., x86, x64, ARM).
    BOOLEAN ImageContainsCode;      // The image contains native executable code.
    union
    {
        UCHAR ImageFlags;
        struct
        {
            UCHAR ComPlusNativeReady : 1;           // The image contains precompiled .NET assembly generated by NGEN (Native Image Generator).
            UCHAR ComPlusILOnly : 1;                // the image contains only Microsoft Intermediate Language (IL) assembly.
            UCHAR ImageDynamicallyRelocated : 1;    // The image was mapped using a random base address rather than the preferred base address.
            UCHAR ImageMappedFlat : 1;              // The image was mapped using a single contiguous region, rather than separate regions for each section.
            UCHAR BaseBelow4gb : 1;                 // The image was mapped using a base address below the 4 GB boundary.
            UCHAR ComPlusPrefer32bit : 1;           // The image prefers to run as a 32-bit process, even on a 64-bit system.
            UCHAR Reserved : 2;
        };
    };
    ULONG LoaderFlags;               // Reserved by ntdll.dll for the Windows loader.
    ULONG ImageFileSize;             // The size of the image, in bytes, including all headers.
    ULONG CheckSum;                  // The image file checksum, from the PE optional header.
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;

typedef struct _SYSTEM_BOOT_ENVIRONMENT_INFORMATION
{
    GUID BootIdentifier;
    FIRMWARE_TYPE FirmwareType;
    union
    {
        ULONGLONG BootFlags;
        struct
        {
            ULONGLONG DbgMenuOsSelection : 1; // REDSTONE4
            ULONGLONG DbgHiberBoot : 1;
            ULONGLONG DbgSoftBoot : 1;
            ULONGLONG DbgMeasuredLaunch : 1;
            ULONGLONG DbgMeasuredLaunchCapable : 1; // 19H1
            ULONGLONG DbgSystemHiveReplace : 1;
            ULONGLONG DbgMeasuredLaunchSmmProtections : 1;
            ULONGLONG DbgMeasuredLaunchSmmLevel : 7; // 20H1
            ULONGLONG DbgBugCheckRecovery : 1; // 24H2
            ULONGLONG DbgFASR : 1;
            ULONGLONG DbgUseCachedBcd : 1;
        };
    };
} SYSTEM_BOOT_ENVIRONMENT_INFORMATION;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
    PVOID Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
    ULONG NumberOfModules;
    _Field_size_(NumberOfModules) RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

NTSTATUS NTAPI NtQuerySystemEnvironmentValueEx(
    _In_ PCUNICODE_STRING VariableName,
    _In_ const GUID* VendorGuid,
    _Out_writes_bytes_opt_(*BufferLength) PVOID Buffer,
    _Inout_ PULONG BufferLength,
    _Out_opt_ PULONG Attributes // EFI_VARIABLE_*
);

NTSTATUS NTAPI RtlGUIDFromString(IN PCUNICODE_STRING GuidString, OUT GUID* Guid);

typedef struct _SYSTEM_SECUREBOOT_INFORMATION
{
    BOOLEAN SecureBootEnabled;
    BOOLEAN SecureBootCapable;
} SYSTEM_SECUREBOOT_INFORMATION, *PSYSTEM_SECUREBOOT_INFORMATION;

NTSTATUS NTAPI NtQuerySystemInformationEx(
    _In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
    _In_reads_bytes_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_opt_(SystemInformationLength) PVOID SystemInformation,
    _In_ ULONG SystemInformationLength,
    _Out_opt_ PULONG ReturnLength
);

typedef enum _SYSTEM_FIRMWARE_TABLE_ACTION
{
    SystemFirmwareTableEnumerate,
    SystemFirmwareTableGet,
    SystemFirmwareTableMax
} SYSTEM_FIRMWARE_TABLE_ACTION;

typedef struct _SYSTEM_FIRMWARE_TABLE_INFORMATION
{
    ULONG ProviderSignature; // (same as the GetSystemFirmwareTable function)
    SYSTEM_FIRMWARE_TABLE_ACTION Action;
    ULONG TableID;
    ULONG TableBufferLength;
    _Field_size_bytes_(TableBufferLength) UCHAR TableBuffer[];
} SYSTEM_FIRMWARE_TABLE_INFORMATION, *PSYSTEM_FIRMWARE_TABLE_INFORMATION;

NTSYSAPI NTSTATUS NTAPI NtDelayExecution(_In_ BOOLEAN Alertable, _In_ PLARGE_INTEGER DelayInterval);

/**
 * The KSYSTEM_TIME structure represents interrupt time, system time, and time zone bias.
 */
typedef struct _KSYSTEM_TIME
{
    ULONG LowPart;
    LONG High1Time;
    LONG High2Time;
} KSYSTEM_TIME, *PKSYSTEM_TIME;

/**
 * PROCESSOR_FEATURE_MAX defines the maximum number of processor feature flags
 * that may be reported by the system.
 */
#define PROCESSOR_FEATURE_MAX 64

/**
 * The ALTERNATIVE_ARCHITECTURE_TYPE enumeration specifies the hardware
 * architecture variant used by the system.
 *
 * \remarks NEC98x86 represents the NEC PC-98 architecture,
 * supported only on very early Windows releases.
 */
typedef enum _ALTERNATIVE_ARCHITECTURE_TYPE
{
    StandardDesign,
    NEC98x86,
    EndAlternatives
} ALTERNATIVE_ARCHITECTURE_TYPE;

/**
 * The KUSER_SHARED_DATA structure contains information shared with user-mode.
 *
 * \sa https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntddk/ns-ntddk-kuser_shared_data
 */
typedef struct _KUSER_SHARED_DATA
{
    //
    // Current low 32-bit of tick count and tick count multiplier.
    //
    // N.B. The tick count is updated each time the clock ticks.
    //

    ULONG TickCountLowDeprecated;
    ULONG TickCountMultiplier;

    //
    // Current 64-bit interrupt time in 100ns units.
    //

    volatile KSYSTEM_TIME InterruptTime;

    //
    // Current 64-bit system time in 100ns units.
    //

    volatile KSYSTEM_TIME SystemTime;

    //
    // Current 64-bit time zone bias.
    //

    volatile KSYSTEM_TIME TimeZoneBias;

    //
    // Support image magic number range for the host system.
    //
    // N.B. This is an inclusive range.
    //

    USHORT ImageNumberLow;
    USHORT ImageNumberHigh;

    //
    // Copy of system root in unicode.
    //
    // N.B. This field must be accessed via the RtlGetNtSystemRoot API for
    //      an accurate result.
    //

    WCHAR NtSystemRoot[260];

    //
    // Maximum stack trace depth if tracing enabled.
    //

    ULONG MaxStackTraceDepth;

    //
    // Crypto exponent value.
    //

    ULONG CryptoExponent;

    //
    // Time zone ID.
    //

    ULONG TimeZoneId;

    //
    // Minimum size of a large page on the system, in bytes.
    //
    // N.B. Returned by GetLargePageMinimum() function.
    //

    ULONG LargePageMinimum;

    //
    // This value controls the Application Impact Telemetry (AIT) Sampling rate.
    //
    // This value determines how frequently the system records AIT events,
    // which are used by the Application Experience and compatibility
    // subsystems to evaluate application behavior, performance, and
    // potential compatibility issues.
    //
    // Lower values increase sampling frequency, while higher values reduce it.
    // The kernel updates this field as part of its internal telemetry and
    // heuristics logic.
    //

    ULONG AitSamplingValue;

    //
    // This value controls Application Compatibility (AppCompat) switchback processing.
    //

    union
    {
        ULONG AppCompatFlag;
        struct
        {
            ULONG SwitchbackEnabled : 1;    // Basic switchback processing
            ULONG ExtendedHeuristics : 1;   // Extended switchback heuristics
            ULONG TelemetryFallback : 1;    // Telemetry-driven fallback
            ULONG Reserved : 29;
        } AppCompatFlags;
    };

    //
    // Current Kernel Root RNG state seed version
    //

    ULONGLONG RNGSeedVersion;

    //
    // This value controls assertion failure handling.
    //
    // Historically (prior to Windows 10), this value was also used by
    // Code Integrity (CI), AppLocker, and related security components to
    // determine the minimum validation requirements for executable images,
    // drivers, and privileged operations.
    //
    // In modern Windows versions, this field is used primarily by the kernel's
    // diagnostic and validation infrastructure to decide how assertion failures
    // should be handled (e.g., logging, debugger break-in, or bugcheck).

    ULONG GlobalValidationRunlevel;

    //
    // Monotonic stamp incremented by the kernel whenever the system's
    // time zone bias value changes.
    //
    // N.B. This field must be accessed via the RtlGetSystemTimeAndBias API for
    //      an accurate result.
    // This value is read before and after accessing the bias fields to determine
    // whether the time zone data changed during the read. If the stamp differs,
    // the caller must re-read the bias values to ensure consistency.
    //

    volatile LONG TimeZoneBiasStamp;

    //
    // The shared collective build number undecorated with C or F.
    // GetVersionEx hides the real number
    //

    ULONG NtBuildNumber;

    //
    // Product type.
    //
    // N.B. This field must be accessed via the RtlGetNtProductType API for
    //      an accurate result.
    //

    NT_PRODUCT_TYPE NtProductType;
    BOOLEAN ProductTypeIsValid;
    BOOLEAN Reserved0[1];

    //
    // Native hardware processor architecture of the running system.
    //
    // N.B. User-mode components read this field to determine the true system
    // architecture, especially in WOW64 scenarios where the process architecture
    // differs from the native one.
    //

    USHORT NativeProcessorArchitecture;

    //
    // The NT Version.
    //
    // N. B. Note that each process sees a version from its PEB, but if the
    //       process is running with an altered view of the system version,
    //       the following two fields are used to correctly identify the
    //       version
    //

    ULONG NtMajorVersion;
    ULONG NtMinorVersion;

    //
    // Processor features.
    //

    BOOLEAN ProcessorFeatures[PROCESSOR_FEATURE_MAX];


    //
    // Reserved fields - do not use.
    //

    ULONG MaximumUserModeAddressDeprecated; // Deprecated, use SystemBasicInformation instead.
    ULONG SystemRangeStartDeprecated; // Deprecated, use SystemRangeStartInformation instead.

    //
    // Time slippage while in debugger.
    //

    volatile ULONG TimeSlip;

    //
    // Alternative system architecture, e.g., NEC PC98xx on x86.
    //

    ALTERNATIVE_ARCHITECTURE_TYPE AlternativeArchitecture;

    //
    // Boot sequence, incremented for each boot attempt by the OS loader.
    //

    ULONG BootId;

    //
    // If the system is an evaluation unit, the following field contains the
    // date and time that the evaluation unit expires. A value of 0 indicates
    // that there is no expiration. A non-zero value is the UTC absolute time
    // that the system expires.
    //

    LARGE_INTEGER SystemExpirationDate;

    //
    // Suite support.
    //
    // N.B. This field must be accessed via the RtlGetSuiteMask API for
    //      an accurate result.
    //

    ULONG SuiteMask;

    //
    // TRUE if a kernel debugger is connected/enabled.
    //

    BOOLEAN KdDebuggerEnabled;

    //
    // Mitigation policies.
    //

    union
    {
        UCHAR MitigationPolicies;
        struct
        {
            UCHAR NXSupportPolicy : 2;
            UCHAR SEHValidationPolicy : 2;
            UCHAR CurDirDevicesSkippedForDlls : 2;
            UCHAR Reserved : 2;
        };
    };

    //
    // Measured duration of a single processor yield, in cycles. This is used by
    // lock packages to determine how many times to spin waiting for a state
    // change before blocking.
    //

    USHORT CyclesPerYield;

    //
    // Current console session Id. Always zero on non-TS systems.
    //
    // N.B. This field must be accessed via the RtlGetActiveConsoleId API for an
    //      accurate result.
    //

    volatile ULONG ActiveConsoleId;

    //
    // Force-dismounts cause handles to become invalid. Rather than always
    // probe handles, a serial number of dismounts is maintained that clients
    // can use to see if they need to probe handles.
    //

    volatile ULONG DismountCount;

    //
    // This field indicates the status of the 64-bit COM+ package on the
    // system. It indicates whether the Intermediate Language (IL) COM+
    // images need to use the 64-bit COM+ runtime or the 32-bit COM+ runtime.
    //

    ULONG ComPlusPackage;

    //
    // Time in tick count for system-wide last user input across all terminal
    // sessions. For MP performance, it is not updated all the time (e.g. once
    // a minute per session). It is used for idle detection.
    //

    ULONG LastSystemRITEventTickCount;

    //
    // Number of physical pages in the system. This can dynamically change as
    // physical memory can be added or removed from a running system.  This
    // cell is too small to hold the non-truncated value on very large memory
    // machines so code that needs the full value should access
    // FullNumberOfPhysicalPages instead.
    //

    ULONG NumberOfPhysicalPages;

    //
    // True if the system was booted in safe boot mode.
    //

    BOOLEAN SafeBootMode;

    //
    // Virtualization flags.
    //

    union
    {
        UCHAR VirtualizationFlags;

#if defined(_ARM64_)

        //
        // N.B. Keep this bitfield in sync with the one in arc.w.
        //

        struct
        {
            UCHAR ArchStartedInEl2 : 1;
            UCHAR QcSlIsSupported : 1;
            UCHAR : 6;
        };

#endif

    };

    //
    // Reserved (available for reuse).
    //

    UCHAR Reserved12[2];

    //
    // This is a packed bitfield that contains various flags concerning
    // the system state. They must be manipulated using interlocked
    // operations.
    //
    // N.B. DbgMultiSessionSku must be accessed via the RtlIsMultiSessionSku
    //      API for an accurate result
    //

    union
    {
        ULONG SharedDataFlags;
        struct
        {
            //
            // The following bit fields are for the debugger only. Do not use.
            // Use the bit definitions instead.
            //

            ULONG DbgErrorPortPresent       : 1;
            ULONG DbgElevationEnabled       : 1;
            ULONG DbgVirtEnabled            : 1;
            ULONG DbgInstallerDetectEnabled : 1;
            ULONG DbgLkgEnabled             : 1;
            ULONG DbgDynProcessorEnabled    : 1;
            ULONG DbgConsoleBrokerEnabled   : 1;
            ULONG DbgSecureBootEnabled      : 1;
            ULONG DbgMultiSessionSku        : 1;
            ULONG DbgMultiUsersInSessionSku : 1;
            ULONG DbgStateSeparationEnabled : 1;
            ULONG DbgSplitTokenEnabled      : 1;
            ULONG DbgShadowAdminEnabled     : 1;
            ULONG SpareBits                 : 19;
        };
    };

    // ... more fields follow, but we don't need them
} KUSER_SHARED_DATA, *PKUSER_SHARED_DATA;

#define SharedUserData ((const KUSER_SHARED_DATA*) 0x7FFE0000UL)

static inline uint64_t ffKSystemTimeToUInt64(const volatile KSYSTEM_TIME* pTime)
{
    #if _WIN64

    return *(uint64_t*) pTime;

    #else

    uint32_t low, high1, high2;

    do {
        high1 = pTime->High1Time;
        low   = pTime->LowPart;
        high2 = pTime->High2Time;
    } while (high1 != high2);

    return ((uint64_t) high1 << 32) | low;
    #endif
}

static inline bool ffIsWindows10OrGreater()
{
    #if FF_WIN81_COMPAT
    return SharedUserData->NtMajorVersion >= 10;
    #else
    return true;
    #endif
}

static inline bool ffIsWindows11OrGreater()
{
    return ffIsWindows10OrGreater() && SharedUserData->NtBuildNumber >= 22000;
}

NTSYSAPI NTSTATUS NTAPI NtOpenProcessToken(
    _In_ HANDLE ProcessHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _Out_ PHANDLE TokenHandle
);
NTSYSAPI NTSTATUS NTAPI NtAdjustPrivilegesToken(
    _In_ HANDLE TokenHandle,
    _In_ BOOLEAN DisableAllPrivileges,
    _In_opt_ PTOKEN_PRIVILEGES NewState,
    _In_ ULONG BufferLength,
    _Out_writes_bytes_to_opt_(BufferLength, *ReturnLength) PTOKEN_PRIVILEGES PreviousState,
    _Out_opt_ PULONG ReturnLength
);
NTSYSAPI NTSTATUS NTAPI NtQueryInformationToken(
    _In_ HANDLE TokenHandle,
    _In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
    _Out_writes_bytes_to_opt_(TokenInformationLength, *ReturnLength) PVOID TokenInformation,
    _In_ ULONG TokenInformationLength,
    _Out_ PULONG ReturnLength
);
#define NtCurrentProcessToken() ((HANDLE)(LONG_PTR)-4) // for NtQueryInformationToken only; Windows 8+

NTSYSAPI NTSTATUS NTAPI NtReadFile(
    _In_ HANDLE FileHandle,
    _In_opt_ HANDLE Event,
    _In_opt_ PIO_APC_ROUTINE ApcRoutine,
    _In_opt_ PVOID ApcContext,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _Out_writes_bytes_(Length) PVOID Buffer,
    _In_ ULONG Length,
    _In_opt_ PLARGE_INTEGER ByteOffset,
    _In_opt_ PULONG Key
);

NTSYSAPI NTSTATUS NTAPI NtCreateEvent(
    _Out_ PHANDLE EventHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_ EVENT_TYPE EventType,
    _In_ BOOLEAN InitialState
);
