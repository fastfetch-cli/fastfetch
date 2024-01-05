#include "diskio.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/IOBSD.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOBlockStorageDriver.h>
#include <IOKit/storage/IOStorageDeviceCharacteristics.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

const char* ffDiskIOGetIoCounters(FFlist* result, FFDiskIOOptions* options)
{
    FF_IOOBJECT_AUTO_RELEASE io_iterator_t iterator = 0;
    if (IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching(kIOMediaClass), &iterator) != KERN_SUCCESS)
        return "IOServiceGetMatchingServices() failed";

    io_registry_entry_t registryEntry;
    while ((registryEntry = IOIteratorNext(iterator)) != IO_OBJECT_NULL)
    {
        FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t entryPartition = registryEntry;

        io_name_t deviceName;
        if (IORegistryEntryGetName(registryEntry, deviceName) != KERN_SUCCESS)
            continue;

        if (options->namePrefix.length && strncmp(deviceName, options->namePrefix.chars, options->namePrefix.length) != 0)
            continue;

        FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t entryDriver = 0;
        if (IORegistryEntryGetParentEntry(entryPartition, kIOServicePlane, &entryDriver) != KERN_SUCCESS)
            continue;

        if (!IOObjectConformsTo(entryDriver, kIOBlockStorageDriverClass)) // physical disk only
            continue;

        FF_CFTYPE_AUTO_RELEASE CFDictionaryRef statistics = IORegistryEntryCreateCFProperty(entryDriver, CFSTR(kIOBlockStorageDriverStatisticsKey), kCFAllocatorDefault, kNilOptions);
        if (!statistics)
            continue;

        FFDiskIOResult* device = (FFDiskIOResult*) ffListAdd(result);
        ffStrbufInitS(&device->name, deviceName);
        ffStrbufInit(&device->devPath);

        ffCfDictGetInt64(statistics, CFSTR(kIOBlockStorageDriverStatisticsBytesReadKey), (int64_t*) &device->bytesRead);
        ffCfDictGetInt64(statistics, CFSTR(kIOBlockStorageDriverStatisticsBytesWrittenKey), (int64_t*) &device->bytesWritten);
        ffCfDictGetInt64(statistics, CFSTR(kIOBlockStorageDriverStatisticsReadsKey), (int64_t*) &device->readCount);
        ffCfDictGetInt64(statistics, CFSTR(kIOBlockStorageDriverStatisticsWritesKey), (int64_t*) &device->writeCount);

        FF_CFTYPE_AUTO_RELEASE CFStringRef bsdName = IORegistryEntryCreateCFProperty(entryPartition, CFSTR(kIOBSDNameKey), kCFAllocatorDefault, kNilOptions);
        if (bsdName)
        {
            ffCfStrGetString(bsdName, &device->devPath);
            ffStrbufPrependS(&device->devPath, "/dev/");
        }
    }

    return NULL;
}
