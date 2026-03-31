#include "physicaldisk.h"
#include "common/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/IOBSD.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOBlockStorageDriver.h>
#include <IOKit/storage/IOStorageDeviceCharacteristics.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>
#ifdef MAC_OS_X_VERSION_10_15
#    include <IOKit/storage/nvme/NVMeSMARTLibExternal.h>
#endif

#ifdef MAC_OS_X_VERSION_10_15
static inline void wrapIoDestroyPlugInInterface(IOCFPlugInInterface*** pluginInf) {
    assert(pluginInf);
    if (*pluginInf) {
        IODestroyPlugInInterface(*pluginInf);
    }
}
#endif

static const char* detectSsdTemp(io_service_t entryPhysical, double* temp) {
#ifdef MAC_OS_X_VERSION_10_15
    __attribute__((__cleanup__(wrapIoDestroyPlugInInterface))) IOCFPlugInInterface** pluginInf = NULL;
    int32_t score;
    if (IOCreatePlugInInterfaceForService(entryPhysical, kIONVMeSMARTUserClientTypeID, kIOCFPlugInInterfaceID, &pluginInf, &score) != kIOReturnSuccess) {
        return "IOCreatePlugInInterfaceForService() failed";
    }

    IONVMeSMARTInterface** smartInf = NULL;
    if ((*pluginInf)->QueryInterface(pluginInf, CFUUIDGetUUIDBytes(kIONVMeSMARTInterfaceID), (LPVOID) &smartInf) != kIOReturnSuccess) {
        return "QueryInterface() failed";
    }

    NVMeSMARTData smartData;
    const char* error = NULL;
    if ((*smartInf)->SMARTReadData(smartInf, &smartData) == kIOReturnSuccess) {
        *temp = smartData.TEMPERATURE - 273;
    } else {
        error = "SMARTReadData() failed";
    }

    (*pluginInf)->Release(smartInf);
    return error;
#else
    return "No support for old MacOS version";
#endif
}

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options) {
    FF_IOOBJECT_AUTO_RELEASE io_iterator_t iterator = 0;
    if (IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching(kIOBlockStorageDriverClass), &iterator) != KERN_SUCCESS) {
        return "IOServiceGetMatchingServices() failed";
    }

    io_registry_entry_t registryEntry;
    while ((registryEntry = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
        FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t entryDriver = registryEntry;

        FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t entryMedia = IO_OBJECT_NULL;
        if (IORegistryEntryGetChildEntry(entryDriver, kIOServicePlane, &entryMedia) != KERN_SUCCESS) {
            continue;
        }

        FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t entryPhysical = 0;
        if (IORegistryEntryGetParentEntry(entryDriver, kIOServicePlane, &entryPhysical) != KERN_SUCCESS) {
            continue;
        }

        io_name_t deviceName;
        if (IORegistryEntryGetName(entryMedia, deviceName) != KERN_SUCCESS) {
            continue;
        }

        if (options->namePrefix.length && strncmp(deviceName, options->namePrefix.chars, options->namePrefix.length) != 0) {
            continue;
        }

        FF_STRBUF_AUTO_DESTROY interconnect = ffStrbufCreate();
        FFPhysicalDiskType diskType = FF_PHYSICALDISK_TYPE_NONE;
        FF_CFTYPE_AUTO_RELEASE CFDictionaryRef protocolCharacteristics = IORegistryEntryCreateCFProperty(entryPhysical, CFSTR(kIOPropertyProtocolCharacteristicsKey), kCFAllocatorDefault, kNilOptions);
        if (protocolCharacteristics) {
            if (ffCfDictGetString(protocolCharacteristics, CFSTR(kIOPropertyPhysicalInterconnectTypeKey), &interconnect) == NULL) {
                if (ffStrbufEqualS(&interconnect, kIOPropertyPhysicalInterconnectTypeVirtual)) {
                    diskType |= FF_PHYSICALDISK_TYPE_VIRTUAL;
                    FF_STRBUF_AUTO_DESTROY location = ffStrbufCreate();
                    if (ffCfDictGetString(protocolCharacteristics, CFSTR(kIOPropertyPhysicalInterconnectLocationKey), &location) == NULL) {
                        ffStrbufAppendS(&interconnect, " - ");
                        ffStrbufAppend(&interconnect, &location);
                    }
                }
            }
        }

        FFPhysicalDiskResult* device = (FFPhysicalDiskResult*) ffListAdd(result);
        ffStrbufInit(&device->serial);
        ffStrbufInit(&device->revision);
        ffStrbufInitS(&device->name, deviceName);
        ffStrbufInit(&device->devPath);
        ffStrbufInitMove(&device->interconnect, &interconnect);
        device->type = diskType;
        device->size = 0;
        device->temperature = FF_PHYSICALDISK_TEMP_UNSET;

        FF_CFTYPE_AUTO_RELEASE CFBooleanRef removable = IORegistryEntryCreateCFProperty(entryMedia, CFSTR(kIOMediaRemovableKey), kCFAllocatorDefault, kNilOptions);
        if (removable) {
            device->type |= CFBooleanGetValue(removable) ? FF_PHYSICALDISK_TYPE_REMOVABLE : FF_PHYSICALDISK_TYPE_FIXED;
        }

        FF_CFTYPE_AUTO_RELEASE CFBooleanRef writable = IORegistryEntryCreateCFProperty(entryMedia, CFSTR(kIOMediaWritableKey), kCFAllocatorDefault, kNilOptions);
        if (writable) {
            device->type |= CFBooleanGetValue(writable) ? FF_PHYSICALDISK_TYPE_READWRITE : FF_PHYSICALDISK_TYPE_READONLY;
        }

        FF_CFTYPE_AUTO_RELEASE CFStringRef bsdName = IORegistryEntryCreateCFProperty(entryMedia, CFSTR(kIOBSDNameKey), kCFAllocatorDefault, kNilOptions);
        if (bsdName) {
            ffCfStrGetString(bsdName, &device->devPath);
            ffStrbufPrependS(&device->devPath, "/dev/");
        }

        FF_CFTYPE_AUTO_RELEASE CFNumberRef mediaSize = IORegistryEntryCreateCFProperty(entryMedia, CFSTR(kIOMediaSizeKey), kCFAllocatorDefault, kNilOptions);
        if (mediaSize) {
            ffCfNumGetInt64(mediaSize, (int64_t*) &device->size);
        } else {
            device->size = 0;
        }

        FF_CFTYPE_AUTO_RELEASE CFDictionaryRef deviceCharacteristics = IORegistryEntryCreateCFProperty(entryPhysical, CFSTR(kIOPropertyDeviceCharacteristicsKey), kCFAllocatorDefault, kNilOptions);
        if (deviceCharacteristics) {
            ffCfDictGetString(deviceCharacteristics, CFSTR(kIOPropertyProductSerialNumberKey), &device->serial);
            ffStrbufTrimSpace(&device->serial);
            ffCfDictGetString(deviceCharacteristics, CFSTR(kIOPropertyProductRevisionLevelKey), &device->revision);
            ffStrbufTrimRightSpace(&device->revision);

            if (!(device->type & FF_PHYSICALDISK_TYPE_VIRTUAL)) {
                CFStringRef mediumType = (CFStringRef) CFDictionaryGetValue(deviceCharacteristics, CFSTR(kIOPropertyMediumTypeKey));
                if (mediumType) {
                    if (CFStringCompare(mediumType, CFSTR(kIOPropertyMediumTypeSolidStateKey), 0) == 0) {
                        device->type |= FF_PHYSICALDISK_TYPE_SSD;
                    } else if (CFStringCompare(mediumType, CFSTR(kIOPropertyMediumTypeRotationalKey), 0) == 0) {
                        device->type |= FF_PHYSICALDISK_TYPE_HDD;
                    }
                }
            }
        }

#ifdef MAC_OS_X_VERSION_10_15
        if (!(device->type & FF_PHYSICALDISK_TYPE_VIRTUAL) && options->temp) {
            FF_CFTYPE_AUTO_RELEASE CFBooleanRef nvmeSMARTCapable = IORegistryEntryCreateCFProperty(entryPhysical, CFSTR(kIOPropertyNVMeSMARTCapableKey), kCFAllocatorDefault, kNilOptions);
            if (nvmeSMARTCapable && CFBooleanGetValue(nvmeSMARTCapable)) {
                detectSsdTemp(entryPhysical, &device->temperature);
            }
        }
#endif
    }

    return NULL;
}
