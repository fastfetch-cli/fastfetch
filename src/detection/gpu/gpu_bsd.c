#include "gpu_driver_specific.h"

#include "common/io/io.h"
#include "common/properties.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"

#include <dev/pci/pcireg.h>
#include <sys/pciio.h>
#include <fcntl.h>

const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus)
{
    FF_AUTO_CLOSE_FD int fd = open("/dev/pci", O_RDONLY, 0);
    struct pci_conf confs[128];
    struct pci_match_conf match = {
        .pc_class = PCIC_DISPLAY,
        .flags = PCI_GETCONF_MATCH_CLASS,
    };
    struct pci_conf_io pcio = {
        .pat_buf_len = sizeof(match),
        .num_patterns = 1,
        .patterns = &match,
        .match_buf_len = sizeof(confs),
        .matches = confs,
    };

    if (ioctl(fd, PCIOCGETCONF, &pcio) < 0)
        return "ioctl(fd, PCIOCGETCONF, &pc) failed";

    if (pcio.status == PCI_GETCONF_ERROR)
        return "ioctl(fd, PCIOCGETCONF, &pc) returned error";

    for (uint32_t i = 0; i < pcio.num_matches; ++i)
    {
        struct pci_conf* pc = &confs[i];

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInitStatic(&gpu->vendor, ffGetGPUVendorString(pc->pc_vendor));
        ffStrbufInit(&gpu->name);
        ffStrbufInitS(&gpu->driver, pc->pd_name);
        ffStrbufInit(&gpu->platformApi);
        gpu->index = FF_GPU_INDEX_UNSET;
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = (pc->pc_sel.pc_domain * 100000ull) + (pc->pc_sel.pc_bus * 1000ull) + (pc->pc_sel.pc_dev * 10ull) + pc->pc_sel.pc_func;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_NVIDIA && (options->temp || options->driverSpecific))
        {
            ffDetectNvidiaGpuInfo(&(FFGpuDriverCondition) {
                .type = FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID,
                .pciBusId = {
                    .domain = (uint32_t) pc->pc_sel.pc_domain,
                    .bus = pc->pc_sel.pc_bus,
                    .device = pc->pc_sel.pc_dev,
                    .func = pc->pc_sel.pc_func,
                },
            }, (FFGpuDriverResult) {
                .index = &gpu->index,
                .temp = options->temp ? &gpu->temperature : NULL,
                .memory = options->driverSpecific ? &gpu->dedicated : NULL,
                .coreCount = options->driverSpecific ? (uint32_t*) &gpu->coreCount : NULL,
                .type = &gpu->type,
                .frequency = &gpu->frequency,
                .coreUsage = &gpu->coreUsage,
                .name = &gpu->name,
            }, "libnvidia-ml.so");
        }

        if (gpu->name.length == 0)
        {
            if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
            {
                char query[32];
                snprintf(query, ARRAY_SIZE(query), "%X,\t%X,", (unsigned) pc->pc_device, (unsigned) pc->pc_revid);
                ffParsePropFileData("libdrm/amdgpu.ids", query, &gpu->name);
            }
            if (gpu->name.length == 0)
            ffGPUFillVendorAndName(pc->pc_subclass, pc->pc_vendor, pc->pc_device, gpu);
        }

        if (gpu->type == FF_GPU_TYPE_UNKNOWN)
        {
            if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_NVIDIA)
            {
                if (ffStrbufStartsWithIgnCaseS(&gpu->name, "GeForce") ||
                    ffStrbufStartsWithIgnCaseS(&gpu->name, "Quadro") ||
                    ffStrbufStartsWithIgnCaseS(&gpu->name, "Tesla"))
                    gpu->type = FF_GPU_TYPE_DISCRETE;
            }
            else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_MTHREADS)
            {
                if (ffStrbufStartsWithIgnCaseS(&gpu->name, "MTT "))
                    gpu->type = FF_GPU_TYPE_DISCRETE;
            }
            else if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_INTEL)
            {
                // 0000:00:02.0 is reserved for Intel integrated graphics
                gpu->type = gpu->deviceId == 20 ? FF_GPU_TYPE_INTEGRATED : FF_GPU_TYPE_DISCRETE;
            }
        }
    }

    return NULL;
}
