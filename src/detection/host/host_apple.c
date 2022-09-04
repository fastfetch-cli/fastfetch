#include "host.h"
#include "common/settings.h"

void ffDetectHostImpl(FFHostResult* host)
{
    ffStrbufInit(&host->productName);

    ffStrbufInitA(&host->productFamily, 0);
    ffStrbufInitA(&host->sysVendor, 0);
    ffStrbufInitA(&host->productVersion, 0);
    ffStrbufInitA(&host->productSku, 0);
    ffStrbufInitA(&host->biosDate, 0);
    ffStrbufInitA(&host->biosRelease, 0);
    ffStrbufInitA(&host->biosVendor, 0);
    ffStrbufInitA(&host->biosVersion, 0);
    ffStrbufInitA(&host->boardName, 0);
    ffStrbufInitA(&host->boardVendor, 0);
    ffStrbufInitA(&host->boardVersion, 0);
    ffStrbufInitA(&host->chassisType, 0);
    ffStrbufInitA(&host->chassisVendor, 0);
    ffStrbufInitA(&host->chassisVersion, 0);

    ffSettingsGetAppleProperty("hw.model", &host->productName);
}
