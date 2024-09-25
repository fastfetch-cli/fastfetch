#include "gpu.h"
#include "common/properties.h"
#include "common/io/io.h"
#include "common/processing.h"

const char* ffDetectGPUImpl(FF_MAYBE_UNUSED const FFGPUOptions* options, FFlist* gpus)
{
    // SunOS requires root permission to query PCI device list, except `/usr/bin/scanpci`
    // Same behavior can be observed with `cp $(which scanpci) /tmp/ && /tmp/scanpci`

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    const char* error = ffProcessAppendStdOut(&buffer, (char* const[]) {
        "scanpci",
        "-v",
        NULL,
    });
    if (error)
        return error;

    if (!ffStrbufStartsWithS(&buffer, "\npci "))
        return "Invalid scanpci result";

    // pci bus 0x0000 cardnum 0x00 function 0x00: vendor 0x1414 device 0x008e
    //  Device unknown
    //  CardVendor 0x0000 card 0x0000 (Card unknown)
    //   STATUS    0x0010  COMMAND 0x0007
    //   CLASS     0x03 0x02 0x00  REVISION 0x00
    //   BIST      0x00  HEADER 0x00  LATENCY 0x00  CACHE 0x00
    //   MAX_LAT   0x00  MIN_GNT 0x00  INT_PIN 0x00  INT_LINE 0x00

    for (
        const char* pclass = strstr(buffer.chars, "\n  CLASS     0x03 ");
        pclass;
        pclass = strstr(pclass, "\n  CLASS     0x03 ")
    )
    {
        // find the start of device entry
        const char* pstart = memrchr(buffer.chars, '\n', (size_t) (pclass - buffer.chars));
        while (pstart[1] != 'p')
            pstart = memrchr(buffer.chars, '\n', (size_t) (pstart - buffer.chars - 1));
        ++pstart;

        uint32_t vendorId, deviceId;
        if (sscanf(pstart, "pci %*[^:]: vendor %x device %x",
            &vendorId, &deviceId) != 2)
            return "PCI info not found, invalid scanpci result";

        pclass += strlen("\n  CLASS     0x03 ");
        uint32_t subclass = (uint32_t) strtoul(pclass, NULL, 16);
        pclass += strlen("0x02 0x00  REVISION ");
        uint32_t revision = (uint32_t) strtoul(pclass, NULL, 16);

        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);
        ffStrbufInitStatic(&gpu->vendor, ffGetGPUVendorString(vendorId));
        ffStrbufInit(&gpu->name);
        ffStrbufInit(&gpu->driver);
        ffStrbufInit(&gpu->platformApi);
        gpu->index = FF_GPU_INDEX_UNSET;
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->coreUsage = FF_GPU_CORE_USAGE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->deviceId = 0;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;

        if (gpu->vendor.chars == FF_GPU_VENDOR_NAME_AMD)
        {
            char query[32];
            snprintf(query, sizeof(query), "%X,\t%X,", (unsigned) deviceId, (unsigned) revision);
            ffParsePropFileData("libdrm/amdgpu.ids", query, &gpu->name);
        }

        if (gpu->name.length == 0)
        {
            ffGPUFillVendorAndName((uint8_t) subclass, (uint16_t) vendorId, (uint16_t) deviceId, gpu);
        }
    }

    return NULL;
}
