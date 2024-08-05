#pragma once

// DISCLAIMER:
// THIS FILE IS CREATED FROM SCRATCH, BY READING THE OFFICIAL NVML API
// DOCUMENTATION REFERENCED BELOW, IN ORDER TO MAKE FASTFETCH MIT COMPLIANT.

// https://docs.nvidia.com/deploy/nvml-api/group__nvmlDeviceStructs.html
#define NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE 32
#define NVML_DEVICE_PCI_BUS_ID_BUFFER_V2_SIZE 16
#define NVML_DEVICE_NAME_V2_BUFFER_SIZE 96

typedef enum { NVML_SUCCESS = 0 } nvmlReturn_t;
typedef struct nvmlDevice_t* nvmlDevice_t;

// https://docs.nvidia.com/deploy/nvml-api/structnvmlPciInfo__t.html
// PCI information about a GPU device
typedef struct {
    // The legacy tuple domain:bus:device.function PCI identifier (& NULL terminator)
    char busIdLegacy[NVML_DEVICE_PCI_BUS_ID_BUFFER_V2_SIZE];
    // The PCI domain on which the device's bus resides, 0 to 0xffffffff
    unsigned int domain;
    // The bus on which the device resides, 0 to 0xff
    unsigned int bus;
    // The device's id on the bus, 0 to 31
    unsigned int device;
    // The combined 16-bit device id and 16-bit vendor id
    unsigned int pciDeviceId;
    // The 32-bit Sub System Device ID
    unsigned int pciSubSystemId;
    // The tuple domain:bus:device.function PCI identifier (& NULL terminator)
    char busId[NVML_DEVICE_PCI_BUS_ID_BUFFER_SIZE];
} nvmlPciInfo_t;

// https://docs.nvidia.com/deploy/nvml-api/group__nvmlDeviceEnumvs.html#group__nvmlDeviceEnumvs_1g2650b526841fa38b8f293c2d509a1de0
// Temperature sensors
typedef enum {
    // Temperature sensor for the GPU die
    NVML_TEMPERATURE_GPU = 0,
    NVML_TEMPERATURE_COUNT,
} nvmlTemperatureSensors_t;

// https://docs.nvidia.com/deploy/nvml-api/structnvmlMemory__v2__t.html#structnvmlMemory__v2__t
// Memory allocation information for a device (v2)
typedef struct {
    // Structure format version (must be 2)
    unsigned int version;
    // Total physical device memory (in bytes)
    unsigned long long total;
    // Device memory (in bytes) reserved for system use (driver or firmware)
    unsigned long long reserved;
    // Unallocated device memory (in bytes)
    unsigned long long free;
    // Allocated device memory (in bytes)
    unsigned long long used;
} nvmlMemory_v2_t;
// https://github.com/NVIDIA/nvidia-settings/issues/78#issuecomment-1012837988
enum { nvmlMemory_v2 = (unsigned int)(sizeof(nvmlMemory_v2_t) | (2 << 24U)) };

// https://docs.nvidia.com/deploy/nvml-api/group__nvmlDeviceEnumvs.html#group__nvmlDeviceEnumvs_1g805c0647be9996589fc5e3f6ff680c64
// Clock types
typedef enum {
    // Graphics clock domain
    NVML_CLOCK_GRAPHICS = 0,
    // SM clock domain
    NVML_CLOCK_SM = 1,
    // Memory clock domain
    NVML_CLOCK_MEM = 2,
    // Video encoder/decoder clock domain
    NVML_CLOCK_VIDEO = 3,
    // Count of clock types
    NVML_CLOCK_COUNT,
} nvmlClockType_t;

// https://docs.nvidia.com/deploy/nvml-api/group__nvmlDeviceEnumvs.html#group__nvmlDeviceEnumvs_1gfa6b01990b212f7b49089b7158eafd2b
// The Brand of the GPU
typedef enum {
    NVML_BRAND_UNKNOWN = 0,
    NVML_BRAND_QUADRO = 1,
    NVML_BRAND_TESLA = 2,
    NVML_BRAND_NVS = 3,
    NVML_BRAND_GRID = 4,
    NVML_BRAND_GEFORCE = 5,
    NVML_BRAND_TITAN = 6,
    NVML_BRAND_NVIDIA_VAPPS = 7,
    NVML_BRAND_NVIDIA_VPC = 8,
    NVML_BRAND_NVIDIA_VCS = 9,
    NVML_BRAND_NVIDIA_VWS = 10,
    NVML_BRAND_NVIDIA_CLOUD_GAMING = 11,
    NVML_BRAND_NVIDIA_VGAMING = NVML_BRAND_NVIDIA_CLOUD_GAMING,
    NVML_BRAND_QUADRO_RTX = 12,
    NVML_BRAND_NVIDIA_RTX = 13,
    NVML_BRAND_NVIDIA = 14,
    NVML_BRAND_GEFORCE_RTX = 15,
    NVML_BRAND_TITAN_RTX = 16,
    NVML_BRAND_COUNT,
} nvmlBrandType_t;

// https://docs.nvidia.com/deploy/nvml-api/structnvmlUtilization__t.html#structnvmlUtilization__t
// Utilization information for a device.
typedef struct
{
    // Percent of time over the past second during which one or more kernels was executing on the GPU
    unsigned int gpu;
    // Percent of time over the past second during which global (device) memory was being read or written
    unsigned int memory;
} nvmlUtilization_t;

// https://docs.nvidia.com/deploy/nvml-api/group__nvmlInitializationAndCleanup.html#group__nvmlInitializationAndCleanup
// Initialize NVML, but don't initialize any GPUs yet
nvmlReturn_t nvmlInit_v2(void);
// Shut down NVML by releasing all GPU resources previously allocated with nvmlInit_v2()
nvmlReturn_t nvmlShutdown(void);

// https://docs.nvidia.com/deploy/nvml-api/group__nvmlDeviceQueries.html
// Retrieves the number of compute devices in the system. A compute device is a single GPU
extern nvmlReturn_t nvmlDeviceGetCount_v2(unsigned int* deviceCount);
// Acquire the handle for a particular device, based on its index
extern nvmlReturn_t nvmlDeviceGetHandleByIndex_v2(unsigned int index, nvmlDevice_t* device);
// Acquire the handle for a particular device, based on its PCI bus id
extern nvmlReturn_t nvmlDeviceGetHandleByPciBusId_v2(const char* pciBusId, nvmlDevice_t* device);
// Retrieves the PCI attributes of this device
extern nvmlReturn_t nvmlDeviceGetPciInfo_v3(nvmlDevice_t device, nvmlPciInfo_t* pci);
// Retrieves the current temperature readings for the device, in degrees C
extern nvmlReturn_t nvmlDeviceGetTemperature(nvmlDevice_t device, nvmlTemperatureSensors_t sensorType, unsigned int* temp);
// Retrieves the amount of used, free, reserved and total memory available on the device, in bytes. The reserved amount is supported on version 2 only
extern nvmlReturn_t nvmlDeviceGetMemoryInfo_v2(nvmlDevice_t device, nvmlMemory_v2_t* memory);
// Gets the device's core count
extern nvmlReturn_t nvmlDeviceGetNumGpuCores(nvmlDevice_t device, unsigned int* numCores);
// Retrieves the maximum clock speeds for the device
extern nvmlReturn_t nvmlDeviceGetMaxClockInfo(nvmlDevice_t device, nvmlClockType_t type, unsigned int* clock);
// Retrieves the brand of this device
extern nvmlReturn_t nvmlDeviceGetBrand(nvmlDevice_t device, nvmlBrandType_t* type);
// Retrieves the current utilization rates for the device
extern nvmlReturn_t nvmlDeviceGetUtilizationRates(nvmlDevice_t device, nvmlUtilization_t *utilization);
// Retrieves the name of this device.
extern nvmlReturn_t nvmlDeviceGetName(nvmlDevice_t device, char *name, unsigned int length);
