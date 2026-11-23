#include "cpu.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/windows/registry.h"
#include "common/windows/nt.h"
#include "common/mallocHelper.h"
#include "common/smbios.h"

#include <windows.h>
#include <winioctl.h>
#include "common/windows/perflib_.h"
#include "common/windows/nt.h"
#include "common/time.h"
#include "3rdparty/pawnio/pawnio_resource.h"
#include <wchar.h>

#if defined(__x86_64__)

// The signed IntelMSR module is embedded as an RCDATA resource (see pawnio.rc).
// It comes from PawnIO.Modules 0.2.12, LGPL-2.1-or-later; its source and license text
// are shipped in src/3rdparty/pawnio/.

    #define FF_PAWNIO_DEVICE_TYPE 41394
    #define FF_PAWNIO_IOCTL_LOAD ((DWORD) CTL_CODE(FF_PAWNIO_DEVICE_TYPE, 0x821, METHOD_BUFFERED, FILE_ANY_ACCESS))
    #define FF_PAWNIO_IOCTL_EXECUTE ((DWORD) CTL_CODE(FF_PAWNIO_DEVICE_TYPE, 0x841, METHOD_BUFFERED, FILE_ANY_ACCESS))
    #define FF_PAWNIO_FN_NAME_LENGTH 32

enum : uint32_t {
    MSR_IA32_THERM_STATUS = 0x0000019c,
    MSR_IA32_TEMPERATURE_TARGET = 0x000001a2,
    MSR_TURBO_RATIO_LIMIT = 0x000001ad,
    MSR_IA32_PACKAGE_THERM_STATUS = 0x000001b1,
};

// Open the driver directly so we can log the exact failure reason.
static HANDLE pawnIoOpenDevice(void) {
    const HANDLE handle = CreateFileW(L"\\\\.\\PawnIO", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (handle != INVALID_HANDLE_VALUE) {
        return handle;
    }

    #ifndef NDEBUG
    const DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
        FF_DEBUG("PawnIO device not found; the driver is probably not installed");
    } else if (error == ERROR_ACCESS_DENIED) {
        FF_DEBUG("PawnIO device access denied; elevation is required");
    } else {
        FF_DEBUG("Failed to open PawnIO device: %s", ffDebugWin32Error(error));
    }
    #endif
    return INVALID_HANDLE_VALUE;
}

static bool pawnIoLoadModule(HANDLE handle) {
    const HRSRC resource = FindResourceW(nullptr, MAKEINTRESOURCEW(FF_PAWNIO_INTEL_MSR_RC_ID), RT_RCDATA);
    if (!resource) {
        FF_DEBUG("PawnIO module resource not found: %s", ffDebugWin32Error(GetLastError()));
        return false;
    }

    const HGLOBAL global = LoadResource(nullptr, resource);
    if (!global) {
        FF_DEBUG("LoadResource(PawnIO module) failed: %s", ffDebugWin32Error(GetLastError()));
        return false;
    }

    const DWORD blobSize = SizeofResource(nullptr, resource);
    const void* const blob = LockResource(global);
    if (blobSize == 0 || !blob) {
        FF_DEBUG("PawnIO module resource is empty");
        return false;
    }

    DWORD bytesReturned = 0;
    if (!DeviceIoControl(handle, FF_PAWNIO_IOCTL_LOAD, (void*) blob, blobSize, nullptr, 0, &bytesReturned, nullptr)) {
        FF_DEBUG("PawnIO module load failed: %s", ffDebugWin32Error(GetLastError()));
        return false;
    }
    return true;
}

static bool pawnIoExecute(HANDLE handle, const char* name, uint64_t msr, uint64_t* value) {
    if (strlen(name) >= FF_PAWNIO_FN_NAME_LENGTH) {
        return false;
    }

    char input[FF_PAWNIO_FN_NAME_LENGTH + sizeof(uint64_t)] = {};
    memcpy(input, name, strlen(name));
    memcpy(input + FF_PAWNIO_FN_NAME_LENGTH, &msr, sizeof(msr));

    DWORD bytesReturned = 0;
    if (!DeviceIoControl(handle, FF_PAWNIO_IOCTL_EXECUTE, input, sizeof(input), value, sizeof(*value), &bytesReturned, nullptr)) {
        FF_DEBUG("PawnIO %s() failed: %s", name, ffDebugWin32Error(GetLastError()));
        return false;
    }
    return bytesReturned == sizeof(*value);
}

static double pawnIoReadCoreTemp(HANDLE handle, uint16_t tjmax) {
    KAFFINITY newMask = 0b11;
    NTSTATUS status = NtSetInformationThread(NtCurrentThread(), ThreadAffinityMask, &newMask, sizeof(newMask));
    if (!NT_SUCCESS(status)) {
        FF_DEBUG("Failed to set thread affinity mask: %s", ffDebugNtError(status));
    }

    // Changing the affinity mask does not migrate an already running thread immediately.
    ffTimeSleep(0);

    uint64_t value = 0;
    const bool success = pawnIoExecute(handle, "ioctl_read_msr", MSR_IA32_THERM_STATUS, &value);

    if (!success || !(value & (UINT64_C(1) << 31))) {
        return FF_CPU_TEMP_UNSET;
    }

    const uint32_t dts = (uint32_t) ((value >> 16) & 0x7f);
    if (dts >= 0x7f) {
        return FF_CPU_TEMP_UNSET;
    }

    return (double) tjmax - dts;
}

static bool detectTemperatureByPawnIo(HANDLE handle, const FFCPUResult* cpu, double* result) {
    if (!ffStrbufEqualS(&cpu->vendor, "GenuineIntel")) {
        return false;
    }

    uint64_t target = 0, package = 0;
    if (!pawnIoExecute(handle, "ioctl_read_msr", MSR_IA32_TEMPERATURE_TARGET, &target) ||
        !pawnIoExecute(handle, "ioctl_read_msr", MSR_IA32_PACKAGE_THERM_STATUS, &package)) {
        return false;
    }

    uint16_t tjmax = (uint16_t) ((target >> 16) & 0xff);
    if (tjmax < 40 || tjmax > 125) {
        tjmax = 100;
    }

    double temperature = pawnIoReadCoreTemp(handle, tjmax);
    if (temperature == FF_CPU_TEMP_UNSET && package & (UINT64_C(1) << 31)) {
        const uint32_t dts = (uint32_t) ((package >> 16) & 0x7f);
        if (dts < 0x7f) {
            temperature = (double) tjmax - dts;
        }
    }

    if (temperature == FF_CPU_TEMP_UNSET) {
        return false;
    }

    *result = temperature;
    FF_DEBUG("PawnIO CPU temperature: %.1f C (tjmax %u, core 0)", temperature, tjmax);
    return true;
}

// IA32_TURBO_RATIO_LIMIT[7:0] is the 1-core P-core turbo ratio. Intel uses a 100 MHz bus clock.
static bool detectFrequencyMaxByPawnIo(HANDLE handle, FFCPUResult* cpu) {
    uint64_t value = 0;
    if (!pawnIoExecute(handle, "ioctl_read_msr", MSR_TURBO_RATIO_LIMIT, &value)) {
        return false;
    }

    const uint32_t ratio = (uint32_t) (value & 0xff);
    if (ratio < 4 || ratio > 128) {
        return false;
    }

    cpu->frequencyMax = ratio * 100;
    FF_DEBUG("PawnIO CPU max frequency: %u MHz (MSR 0x1AD ratio %u)", cpu->frequencyMax, ratio);
    return true;
}

#endif

static inline void ffPerfCloseQueryHandle(HANDLE* phQuery) {
    if (*phQuery != nullptr) {
        PerfCloseQueryHandle(*phQuery);
        *phQuery = nullptr;
    }
}

const char* detectThermalTemp(const FFCPUOptions* options, double* result) {
    struct FFPerfQuerySpec {
        PERF_COUNTER_IDENTIFIER Identifier;
        WCHAR Name[16];
    } querySpec = {
        .Identifier = {
            // Thermal Zone Information
            // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Perflib\_V2Providers\{383487a6-3676-4870-a4e7-d45b30c35629}\{52bc5412-dac2-449c-8bc2-96443888fe6b}
            .CounterSetGuid = { 0x52bc5412, 0xdac2, 0x449c, { 0x8b, 0xc2, 0x96, 0x44, 0x38, 0x88, 0xfe, 0x6b } },
            .Size = sizeof(querySpec),
            .CounterId = PERF_WILDCARD_COUNTER,
            .InstanceId = PERF_WILDCARD_COUNTER,
        },
        .Name = L"\\_TZ.CPUZ", // The standard(?) instance name for CPU temperature in the thermal provider
    };

    if (options->tempSensor.length > 0) {
        NTSTATUS status = RtlUTF8ToUnicodeN(querySpec.Name, (ULONG) sizeof(querySpec.Name), nullptr, options->tempSensor.chars, (ULONG) options->tempSensor.length + 1);
        if (!NT_SUCCESS(status)) {
            FF_DEBUG("Invalid temp sensor string: %s", ffDebugNtStatus(status));
            return "Invalid temp sensor string";
        }
    }

    DWORD dataSize = 0;
    DWORD ret = PerfEnumerateCounterSetInstances(nullptr, &querySpec.Identifier.CounterSetGuid, nullptr, 0, &dataSize);
    if (ret != ERROR_NOT_ENOUGH_MEMORY) {
        FF_DEBUG("PerfEnumerateCounterSetInstances() failed: %s", ffDebugWin32Error(ret));
        return "PerfEnumerateCounterSetInstances() failed";
    }

    if (dataSize <= sizeof(PERF_INSTANCE_HEADER)) {
        FF_DEBUG("No `Thermal Zone Information` instances found");
        return "No `Thermal Zone Information` instances found";
    }

    {
        FF_AUTO_FREE PERF_INSTANCE_HEADER* const pHead = malloc(dataSize);
        ret = PerfEnumerateCounterSetInstances(nullptr, &querySpec.Identifier.CounterSetGuid, pHead, dataSize, &dataSize);
        if (ret != ERROR_SUCCESS) {
            FF_DEBUG("PerfEnumerateCounterSetInstances() failed to get instance headers: %s", ffDebugWin32Error(ret));
            return "PerfEnumerateCounterSetInstances() failed to get instance headers";
        }

        PERF_INSTANCE_HEADER* pInstanceHeader = pHead;
        while (1) {
            const wchar_t* instanceName = (const wchar_t*) ((BYTE*) pInstanceHeader + sizeof(*pInstanceHeader));
            if (wcscmp(instanceName, querySpec.Name) == 0) {
                break;
            }

            dataSize -= pInstanceHeader->Size;
            if (dataSize == 0) {
                break;
            }
            pInstanceHeader = (PERF_INSTANCE_HEADER*) ((BYTE*) pInstanceHeader + pInstanceHeader->Size);
        }

        if (dataSize == 0) {
            if (options->tempSensor.length > 0) {
                FF_DEBUG("Unable to find CPU sensor");
                return "Unable to find CPU sensor";
            }

            const wchar_t* instanceName = (const wchar_t*) ((BYTE*) pHead + sizeof(*pHead));
            wcscpy(querySpec.Name, instanceName); // Use the first instance name if the specific one is not found
        }
    }

    [[gnu::cleanup(ffPerfCloseQueryHandle)]]
    HANDLE hQuery = nullptr;

    ret = PerfOpenQueryHandle(nullptr, &hQuery);
    if (ret != ERROR_SUCCESS) {
        FF_DEBUG("PerfOpenQueryHandle() failed: %s", ffDebugWin32Error(ret));
        return "PerfOpenQueryHandle() failed";
    }

    ret = PerfAddCounters(hQuery, &querySpec.Identifier, sizeof(querySpec));
    if (ret != ERROR_SUCCESS) {
        FF_DEBUG("PerfAddCounters() failed: %s", ffDebugWin32Error(ret));
        return "PerfAddCounters() failed";
    }

    if (querySpec.Identifier.Status != ERROR_SUCCESS) {
        FF_DEBUG("PerfAddCounters() reports invalid identifier: %s", ffDebugWin32Error(querySpec.Identifier.Status));
        return "PerfAddCounters() reports invalid identifier";
    }

    ret = PerfQueryCounterData(hQuery, nullptr, 0, &dataSize);
    if (ret != ERROR_NOT_ENOUGH_MEMORY) {
        FF_DEBUG("PerfQueryCounterData(nullptr) failed: %s", ffDebugWin32Error(ret));
        return "PerfQueryCounterData(nullptr) failed";
    }

    if (dataSize <= sizeof(PERF_DATA_HEADER) + sizeof(PERF_COUNTER_HEADER)) { // PERF_ERROR_RETURN, should not happen
        FF_DEBUG("instance doesn't exist");
        return "instance doesn't exist";
    }

    FF_AUTO_FREE PERF_DATA_HEADER* const pDataHeader = malloc(dataSize);

    ret = PerfQueryCounterData(hQuery, pDataHeader, dataSize, &dataSize);
    if (ret != ERROR_SUCCESS) {
        FF_DEBUG("PerfQueryCounterData(pDataHeader) failed: %s", ffDebugWin32Error(ret));
        return "PerfQueryCounterData(pDataHeader) failed";
    }

    PERF_COUNTER_HEADER* pCounterHeader = (PERF_COUNTER_HEADER*) (pDataHeader + 1);
    if (pCounterHeader->dwType != PERF_MULTIPLE_COUNTERS) {
        FF_DEBUG("Invalid counter type");
        return "Invalid counter type";
    }

    PERF_MULTI_COUNTERS* pMultiCounters = (PERF_MULTI_COUNTERS*) (pCounterHeader + 1);
    PERF_COUNTER_DATA* pCounterData = (PERF_COUNTER_DATA*) ((BYTE*) pMultiCounters + pMultiCounters->dwSize);

    for (ULONG iCounter = 0; iCounter != pMultiCounters->dwCounters; iCounter++) {
        if (pCounterData->dwDataSize == sizeof(int32_t)) {
            DWORD* pCounterIds = (DWORD*) (pMultiCounters + 1);
            int32_t value = *(int32_t*) (pCounterData + 1);
            if (value == 0) {
                return "Temperature data is zero";
            }

            switch (pCounterIds[iCounter]) {
                case 0: // Temperature
                    *result = value - 273;
                    break;
                case 3: // High Precision Temperature
                    *result = value / 10.0 - 273;
                    break;
            }
        }

        pCounterData = (PERF_COUNTER_DATA*) ((BYTE*) pCounterData + pCounterData->dwSize);
    }

    return nullptr;
}

// 7.5
typedef struct [[gnu::packed]] FFSmbiosProcessorInfo {
    FFSmbiosHeader Header;

    uint8_t SocketDesignation;     // string
    uint8_t ProcessorType;         // enum
    uint8_t ProcessorFamily;       // enum
    uint8_t ProcessorManufacturer; // string
    uint64_t ProcessorID;          // varies
    uint8_t ProcessorVersion;      // string
    uint8_t Voltage;               // varies
    uint16_t ExternalClock;        // varies
    uint16_t MaxSpeed;             // varies
    uint16_t CurrentSpeed;         // varies
    uint8_t Status;                // varies
    uint8_t ProcessorUpgrade;      // enum

    // 2.1+
    uint16_t L1CacheHandle; // varies
    uint16_t L2CacheHandle; // varies
    uint16_t L3CacheHandle; // varies

    // 2.3+
    uint8_t SerialNumber; // string
    uint8_t AssertTag;    // string
    uint8_t PartNumber;   // string

    // 2.5+
    uint8_t CoreCount;                 // varies
    uint8_t CoreEnabled;               // varies
    uint8_t ThreadCount;               // varies
    uint16_t ProcessorCharacteristics; // bit field

    // 2.6+
    uint16_t ProcessorFamily2; // enum

    // 3.0+
    uint16_t CoreCount2;   // varies
    uint16_t CoreEnabled2; // varies
    uint16_t ThreadCount2; // varies

    // 3.6+
    uint16_t ThreadEnabled; // varies
} FFSmbiosProcessorInfo;

static_assert(offsetof(FFSmbiosProcessorInfo, ThreadEnabled) == 0x30,
    "FFSmbiosProcessorInfo: Wrong struct alignment");

static const char* detectMaxSpeedBySmbios(FFCPUResult* cpu) {
    const FFSmbiosHeaderTable* smbiosTable = ffGetSmbiosHeaderTable();
    if (!smbiosTable) {
        FF_DEBUG("Failed to get SMBIOS data");
        return "Failed to get SMBIOS data";
    }

    const FFSmbiosProcessorInfo* data = (const FFSmbiosProcessorInfo*) (*smbiosTable)[FF_SMBIOS_TYPE_PROCESSOR_INFO];

    if (!data) {
        FF_DEBUG("Processor information is not found in SMBIOS data");
        return "Processor information is not found in SMBIOS data";
    }

    while (data->ProcessorType != 0x03 /*Central Processor*/ || (data->Status & 0b00000111) != 1 /*Enabled*/) {
        data = (const FFSmbiosProcessorInfo*) ffSmbiosNextEntry(&data->Header);
        if (data->Header.Type != FF_SMBIOS_TYPE_PROCESSOR_INFO) {
            FF_DEBUG("No active CPU is found in SMBIOS data");
            return "No active CPU is found in SMBIOS data";
        }
    }

    uint32_t speed = FF_READ_LE(data->MaxSpeed);
    // Sometimes SMBIOS reports invalid value. We assume that max speed is small than 2x of base
    if (speed < cpu->frequencyBase || speed > cpu->frequencyBase * 2) {
        FF_DEBUG("Possible invalid CPU max speed in SMBIOS data. See #800");
        return "Possible invalid CPU max speed in SMBIOS data. See #800";
    }

    cpu->frequencyMax = speed;

    return nullptr;
}

static uint32_t getNumLogicalCores(const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* ptr) {
    uint32_t num = 0;
    for (uint32_t i = 0; i < ptr->Processor.GroupCount; ++i) {
        num += (uint32_t)
#if _WIN64
            __builtin_popcountll
#else
            __builtin_popcountl
#endif
            (ptr->Processor.GroupMask[i].Mask);
    }
    return num;
}

static const char* detectNCores(const FFCPUOptions* options, FFCPUResult* cpu) {
    LOGICAL_PROCESSOR_RELATIONSHIP lpr = RelationAll;
    ULONG length = 0;
    NtQuerySystemInformationEx(SystemLogicalProcessorAndGroupInformation, &lpr, sizeof(lpr), nullptr, 0, &length);
    if (length == 0) {
        FF_DEBUG("GetLogicalProcessorInformationEx(RelationAll, nullptr, &length) failed");
        return "GetLogicalProcessorInformationEx(RelationAll, nullptr, &length) failed";
    }

    FF_AUTO_FREE SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*
        pProcessorInfo = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) malloc(length);

    NTSTATUS status = NtQuerySystemInformationEx(SystemLogicalProcessorAndGroupInformation, &lpr, sizeof(lpr), pProcessorInfo, length, &length);
    if (!NT_SUCCESS(status)) {
        FF_DEBUG("GetLogicalProcessorInformationEx(RelationAll, pProcessorInfo, &length) failed: %s", ffDebugNtStatus(status));
        return "GetLogicalProcessorInformationEx(RelationAll, pProcessorInfo, &length) failed";
    }

    for (
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* ptr = pProcessorInfo;
        (uint8_t*) ptr < ((uint8_t*) pProcessorInfo) + length;
        ptr = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) (((uint8_t*) ptr) + ptr->Size)) {
        if (ptr->Relationship == RelationGroup) {
            for (uint32_t index = 0; index < ptr->Group.ActiveGroupCount; ++index) {
                cpu->coresOnline += ptr->Group.GroupInfo[index].ActiveProcessorCount;
                cpu->coresLogical += ptr->Group.GroupInfo[index].MaximumProcessorCount;
            }
        } else if (ptr->Relationship == RelationProcessorCore) {
            ++cpu->coresPhysical;

            if (options->showPeCoreCount) {
                for (uint32_t i = 0; i < ARRAY_SIZE(cpu->coreTypes); ++i) {
                    if (ptr->Processor.EfficiencyClass + 1 == cpu->coreTypes[i].freq) {
                        cpu->coreTypes[i].count += getNumLogicalCores(ptr);
                        break;
                    } else if (cpu->coreTypes[i].freq == 0) {
                        cpu->coreTypes[i].freq = ptr->Processor.EfficiencyClass + 1;
                        cpu->coreTypes[i].count += getNumLogicalCores(ptr);
                        break;
                    }
                }
            }
        } else if (ptr->Relationship == RelationProcessorPackage) {
            ++cpu->packages;
        } else if (ptr->Relationship == RelationNumaNode) {
            ++cpu->numaNodes;
        }
    }

    return nullptr;
}

static const char* detectByRegistry(FFCPUResult* cpu) {
    FF_AUTO_CLOSE_FD HANDLE hKey = nullptr;
    if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hKey, nullptr)) {
        FF_DEBUG("ffRegOpenKeyForRead() failed for HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0");
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\", &hKey, nullptr) failed";
    }

    if (ffRegReadValues(hKey, 3, (FFRegValueArg[]){
                                     FF_ARG(cpu->name, L"ProcessorNameString"),
                                     FF_ARG(cpu->vendor, L"VendorIdentifier"),
                                     FF_ARG(cpu->frequencyBase, L"~MHz"),
                                 },
            nullptr)) {
        ffStrbufTrimRightSpace(&cpu->vendor);
    } else {
        FF_DEBUG("ffRegReadValues() failed for CPU registry key");
        return "ffRegReadValues() failed for CPU registry key";
    }

    return nullptr;
}

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu) {
    detectNCores(options, cpu);

    const char* error = detectByRegistry(cpu);
    if (error) {
        return error;
    }

    ffCPUDetectByCpuid(cpu);

    if (cpu->frequencyMax == 0) {
        detectMaxSpeedBySmbios(cpu);
    }

    bool pawnIoTemperature = false;
#ifdef __x86_64__
    if (ffStrbufEqualS(&cpu->vendor, "GenuineIntel")) {
        FF_AUTO_CLOSE_FD HANDLE hPawnIo = pawnIoOpenDevice();
        if (hPawnIo != INVALID_HANDLE_VALUE && pawnIoLoadModule(hPawnIo)) {
            detectFrequencyMaxByPawnIo(hPawnIo, cpu);
            if (options->temp) {
                pawnIoTemperature = detectTemperatureByPawnIo(hPawnIo, cpu, &cpu->temperature);
            }
        }
    }
#endif

    if (options->temp && !pawnIoTemperature) {
        detectThermalTemp(options, &cpu->temperature);
    }

    return nullptr;
}
