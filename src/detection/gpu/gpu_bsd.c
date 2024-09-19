#include "gpu_driver_specific.h"

#include "common/io/io.h"
#include "common/properties.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"

#include <dev/pci/pcireg.h>
#include <sys/pciio.h>
#include <fcntl.h>
#include <paths.h>

static bool loadPciIds(FFstrbuf* pciids)
{
    // https://github.com/freebsd/freebsd-src/blob/main/usr.sbin/pciconf/pathnames.h

    ffReadFileBuffer(_PATH_LOCALBASE "/share/pciids/pci.ids", pciids);
    if (pciids->length > 0) return true;

    ffReadFileBuffer(FASTFETCH_TARGET_DIR_USR "/share/pciids/pci.ids", pciids);
    if (pciids->length > 0) return true;

    return false;
}

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

    FF_STRBUF_AUTO_DESTROY pciids = ffStrbufCreate();

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
        gpu->deviceId = ((uint64_t) pc->pc_sel.pc_domain << 6) | ((uint64_t) pc->pc_sel.pc_bus << 4) | ((uint64_t) pc->pc_sel.pc_dev << 2) | pc->pc_sel.pc_func;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
        {
            char query[32];
            snprintf(query, sizeof(query), "%X,\t%X,", (unsigned) pc->pc_device, (unsigned) pc->pc_revid);
            ffParsePropFileData("libdrm/amdgpu.ids", query, &gpu->name);
        }

        FF_STRBUF_AUTO_DESTROY coreName = ffStrbufCreate();
        if (gpu->name.length == 0)
        {
            if (pciids.length == 0)
                loadPciIds(&pciids);
            ffGPUParsePciIds(&pciids, pc->pc_subclass, pc->pc_vendor, pc->pc_device, gpu, &coreName);
        }

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
                .name = options->driverSpecific ? &gpu->name : NULL,
            }, "libnvidia-ml.so");
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
                if ((coreName.chars[0] == 'D' || coreName.chars[0] == 'S') &&
                        coreName.chars[1] == 'G' &&
                        ffCharIsDigit(coreName.chars[2]))
                    gpu->type = FF_GPU_TYPE_DISCRETE; // DG1 / DG2 / SG1
                else
                    gpu->type = FF_GPU_TYPE_INTEGRATED;
            }
        }
    }

    return NULL;
}
