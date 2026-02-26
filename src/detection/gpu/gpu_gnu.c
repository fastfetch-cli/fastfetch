#include "gpu.h"
#include "common/io.h"

#include <hurd.h>
#include <hurd/pci.h>
#include <hurd/paths.h>

enum {
    PCI_VENDOR_ID    = 0x00,
    PCI_DEVICE_ID    = 0x02,
    PCI_REVISION_ID  = 0x08,
    PCI_CLASS_PROG   = 0x09,
    PCI_SUBCLASS     = 0x0a,
    PCI_CLASS_DEVICE = 0x0b,
    PCI_CONF_SIZE    = 0x40,
};

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    int dDomainFd = open(_SERVERS_BUS "/pci/0000", O_RDONLY | O_CLOEXEC);
    if (dDomainFd < 0) return "open(_SERVERS_BUS \"/pci/0000\") failed";

    FF_AUTO_CLOSE_DIR DIR* dirDomain = fdopendir(dDomainFd);
    if (dirDomain == NULL) return "fdopendir(domain) failed";

    struct dirent* busEntry;
    while ((busEntry = readdir(dirDomain)) != NULL)
    {
        if (busEntry->d_type != DT_DIR || busEntry->d_name[0] == '.')
            continue;

        char* endptr;
        uint16_t pciBus = (uint16_t) strtoul(busEntry->d_name, &endptr, 16);
        if (*endptr != '\0') continue;

        int dBusFd = openat(dDomainFd, busEntry->d_name, O_RDONLY | O_CLOEXEC);
        if (dBusFd < 0) continue;

        FF_AUTO_CLOSE_DIR DIR* dirBus = fdopendir(dBusFd);
        if (dirBus == NULL) continue;

        struct dirent* devEntry;
        while ((devEntry = readdir(dirBus)) != NULL)
        {
            if (devEntry->d_type != DT_DIR || devEntry->d_name[0] == '.')
                continue;

            uint8_t pciDev = (uint8_t) strtoul(devEntry->d_name, &endptr, 16);
            if (*endptr != '\0') continue;

            int dDevFd = openat(dBusFd, devEntry->d_name, O_RDONLY | O_CLOEXEC);
            if (dDevFd < 0) continue;

            FF_AUTO_CLOSE_DIR DIR* dirDev = fdopendir(dDevFd);
            if (dirDev == NULL) continue;

            struct dirent* funcEntry;
            while ((funcEntry = readdir(dirDev)) != NULL)
            {
                if (funcEntry->d_type != DT_DIR || funcEntry->d_name[0] == '.')
                    continue;

                uint8_t pciFunc = (uint8_t) strtoul(funcEntry->d_name, &endptr, 16);
                if (*endptr != '\0') continue;

                char subpath[PATH_MAX];
                snprintf(subpath, ARRAY_SIZE(subpath), "%s/%s/%s/%s/config", _SERVERS_BUS "/pci/0000", busEntry->d_name, devEntry->d_name, funcEntry->d_name);

                mach_port_t devicePort = file_name_lookup(subpath, 0, 0);
                if (devicePort == MACH_PORT_NULL)
                    continue;

                mach_msg_type_number_t nread = 0;

                uint8_t data[PCI_CONF_SIZE];
                data_t pData = (data_t) data;
                kern_return_t kr = pci_conf_read(devicePort, 0, &pData, &nread, PCI_CONF_SIZE);
                mach_port_deallocate(mach_task_self(), devicePort);
                if (kr != KERN_SUCCESS || nread < PCI_CONF_SIZE) continue;

                if (pData != (data_t) data)
                {
                    memcpy(data, pData, PCI_CONF_SIZE);
                    vm_deallocate(mach_task_self(), (vm_address_t)pData, nread);
                }

                uint8_t classBase = data[PCI_CLASS_DEVICE];
                if (classBase != 0x03 /*PCI_BASE_CLASS_DISPLAY*/)
                    continue;

                uint8_t classSub = data[PCI_SUBCLASS];
                if (pciFunc > 0 && classSub == 0x80 /*PCI_CLASS_DISPLAY_OTHER*/) // Likely an auxiliary display controller (#2034)
                    continue;

                uint8_t revision = data[PCI_REVISION_ID];
                uint16_t vendorId = data[PCI_VENDOR_ID] | (data[PCI_VENDOR_ID + 1] << 8);
                uint16_t deviceId = data[PCI_DEVICE_ID] | (data[PCI_DEVICE_ID + 1] << 8);

                FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
                ffStrbufInitStatic(&gpu->vendor, ffGPUGetVendorString(vendorId));
                ffStrbufInit(&gpu->name);
                ffStrbufInit(&gpu->driver);
                ffStrbufInitStatic(&gpu->platformApi, "/servers/bus/pci");
                ffStrbufInit(&gpu->memoryType);
                gpu->temperature = FF_GPU_TEMP_UNSET;
                gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
                gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
                gpu->type = FF_GPU_TYPE_UNKNOWN;
                gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
                gpu->deviceId = ffGPUPciAddr2Id(0, pciBus, pciDev, pciFunc);
                gpu->frequency = FF_GPU_FREQUENCY_UNSET;

                if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
                    ffGPUQueryAmdGpuName(deviceId, revision, gpu);

                if (gpu->name.length == 0)
                    ffGPUFillVendorAndName(classSub, vendorId, deviceId, gpu);
            }
        }
    }

    return NULL;
}
