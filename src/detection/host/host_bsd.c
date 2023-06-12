#include "host.h"
#include "common/sysctl.h"

const char* ffDetectHost(FFHostResult* host)
{
    ffStrbufInit(&host->productName);
    ffStrbufInit(&host->productFamily);
    ffStrbufInit(&host->productVersion);
    ffStrbufInit(&host->productSku);
    ffStrbufInit(&host->sysVendor);

    return ffSysctlGetString("hw.fdt.model", &host->productName);
}
