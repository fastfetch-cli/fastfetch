#pragma once

// DISCLAIMER:
// THIS FILE IS CREATED FROM SCRATCH, BY READING THE OFFICIAL MTML API
// DOCUMENTATION REFERENCED BELOW, IN ORDER TO MAKE FASTFETCH MIT COMPLIANT.

#define MTML_API __attribute__((visibility("default")))
#define MTML_DEVICE_PCI_SBDF_BUFFER_SIZE 32
#define MTML_DEVICE_NAME_BUFFER_SIZE 32
#define MTML_DEVICE_UUID_BUFFER_SIZE 48

/**
 * Return values for MTML API calls.
 */
typedef enum
{
    MTML_SUCCESS = 0,
} MtmlReturn;

/**
 * The brand of the device.
 */
typedef enum
{
    MTML_BRAND_MTT = 0, //!< MTT series.
    MTML_BRAND_UNKNOWN, //!< An unknown brand.

    // Keep this on the last line.
    MTML_BRAND_COUNT //!< The number of brands.
} MtmlBrandType;

typedef struct MtmlLibrary MtmlLibrary;
typedef struct MtmlSystem MtmlSystem;
typedef struct MtmlDevice MtmlDevice;
typedef struct MtmlGpu MtmlGpu;
typedef struct MtmlMemory MtmlMemory;

/**
 * PCI information about a device.
 */
typedef struct
{
    char sbdf[MTML_DEVICE_PCI_SBDF_BUFFER_SIZE]; //!< The tuple segment:bus:device.function PCI identifier (&amp; NULL terminator).
    unsigned int segment;                        //!< The PCI segment group(domain) on which the device's bus resides, 0 to 0xffffffff.
    unsigned int bus;                            //!< The bus on which the device resides, 0 to 0xff.
    unsigned int device;                         //!< The device ID on the bus, 0 to 31.
    unsigned int pciDeviceId;                    //!< The combined 16-bit device ID and 16-bit vendor ID.
    unsigned int pciSubsystemId;                 //!< The 32-bit sub system device ID.
    unsigned int busWidth;                       //!< @deprecated This value set to zero.
    float pciMaxSpeed;                           //!< The maximum link speed (transfer rate per lane) of the device. The unit is GT/s.
    float pciCurSpeed;                           //!< The current link speed (transfer rate per lane) of the device. The unit is GT/s.
    unsigned int pciMaxWidth;                    //!< The maximum link width of the device.
    unsigned int pciCurWidth;                    //!< The current link width of the device.
    unsigned int pciMaxGen;                      //!< The maximum supported generation of the device.
    unsigned int pciCurGen;                      //!< The current generation of the device.
    int rsvd[6];                                 //!< Reserved for future extension.
} MtmlPciInfo;

// Retrieves the number of cores of a device.
MtmlReturn MTML_API mtmlDeviceCountGpuCores(const MtmlDevice* device, unsigned int* numCores);
// Retrieves the brand of a device.
MtmlReturn MTML_API mtmlDeviceGetBrand(const MtmlDevice *dev, MtmlBrandType *type);
// Retrieves the index associated with the specified device.
MtmlReturn MTML_API mtmlDeviceGetIndex(const MtmlDevice *dev, unsigned int *index);
// Retrieves the name of a device.
MtmlReturn MTML_API mtmlDeviceGetName(const MtmlDevice *dev, char *name, unsigned int length);
// Retrieves the PCI attributes of a device.
MtmlReturn MTML_API mtmlDeviceGetPciInfo(const MtmlDevice *dev, MtmlPciInfo *pci);
/**
 * Retrieves the UUID of a specified device. The UUID is a hexadecimal string in the
 * form of xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx, where each 'x' is an ASCII character that represents a hexadecimal
 * digit. The UUID is globally unique for every single device thus can be used to identify different devices
 * physically.
 */
MtmlReturn MTML_API mtmlDeviceGetUUID(const MtmlDevice *dev, char *uuid, unsigned int length);
// Initializes a GPU opaque object to represent a specific graphic core on the target device that is designated by its index.
MtmlReturn MTML_API mtmlDeviceInitGpu(const MtmlDevice *dev, MtmlGpu **gpu);
// Initializes a memory opaque object to represent the memory on the target device.
MtmlReturn MTML_API mtmlDeviceInitMemory(const MtmlDevice *dev, MtmlMemory **mem);

// Retrieves the maximum supported clock speed for the device's graphic core.
MtmlReturn MTML_API mtmlGpuGetMaxClock(const MtmlGpu *gpu, unsigned int *clockMhz);
// Retrieves the current temperature readings for the device's graphic core, in degrees Celsius.
MtmlReturn MTML_API mtmlGpuGetTemperature(const MtmlGpu *gpu, unsigned int *temp);
// Retrieves the current utilization rate for the device's graphic core.
MtmlReturn MTML_API mtmlGpuGetUtilization(const MtmlGpu *gpu, unsigned int *utilization);

// Retrieves the number of devices that can be accessed by the library opaque object.
MtmlReturn MTML_API mtmlLibraryCountDevice(const MtmlLibrary *lib, unsigned int *count);
/**
 * Initializes a device opaque object to represent a device that is designated by its index.
 * The index ranges from (0) to (deviceCount - 1), where deviceCount is retrieved from \ref mtmlLibraryCountDevice().
 */
MtmlReturn MTML_API mtmlLibraryInit(MtmlLibrary **lib);
/**
 * Initializes a device opaque object to represent a device that is designated by its index.
 * The index ranges from (0) to (deviceCount - 1), where deviceCount is retrieved from \ref mtmlLibraryCountDevice().
 */
MtmlReturn MTML_API mtmlLibraryInitDeviceByIndex(const MtmlLibrary *lib, unsigned int index, MtmlDevice **dev);
/**
 * Initializes a device opaque object to represent a device that is designated by its PCI Sbdf.
 * The PCI Sbdf format like 00000000:3a:00.0 refer to \ref MtmlPciInfo::sbdf.
 */
MtmlReturn MTML_API mtmlLibraryInitDeviceByPciSbdf(const MtmlLibrary *lib, const char *pciSbdf, MtmlDevice **dev);
// Initializes a MtmlSystem opaque pointer that is bound to a library opaque object.
MtmlReturn MTML_API mtmlLibraryInitSystem(const MtmlLibrary *lib, MtmlSystem **sys);
/**
 * Shuts down the library opaque object that is previously initialized by \ref mtmlLibraryInit() and releases its resources.
 * The \a lib pointer cannot be used anymore after this function returns.
 */
MtmlReturn MTML_API mtmlLibraryShutDown(MtmlLibrary *lib);

// Retrieves the amount of total memory available on the device, in bytes.
MtmlReturn MTML_API mtmlMemoryGetTotal(const MtmlMemory *mem, unsigned long long *total);
// Retrieves the amount of used memory on the device, in bytes.
MtmlReturn MTML_API mtmlMemoryGetUsed(const MtmlMemory *mem, unsigned long long *used);
// Retrieves the current memory utilization rate for the device.
MtmlReturn MTML_API mtmlMemoryGetUtilization(const MtmlMemory *mem, unsigned int *utilization);
