#include "host.h"
#include "common/sysctl.h"

void ffDetectHostImpl(FFHostResult* host)
{
    ffStrbufInit(&host->error);

    ffStrbufInit(&host->productName);
    ffStrbufInit(&host->productFamily);
    ffStrbufInit(&host->productVersion);
    ffStrbufInit(&host->productSku);
    ffStrbufInit(&host->sysVendor);

    ffStrbufAppendS(&host->error, ffSysctlGetString("hw.fdt.model", &host->productName));
}
