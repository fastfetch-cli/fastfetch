#include "host.h"
#include "common/settings.h"
#include <ctype.h>

void ffDetectHostImpl(FFHostResult* host)
{
    //Family

    ffStrbufInit(&host->productFamily);
    ffSettingsGetAndroidProperty("ro.product.device", &host->productFamily);

    //Name

    ffStrbufInit(&host->productName);

    ffSettingsGetAndroidProperty("ro.product.brand", &host->productName);
    if(host->productName.length > 0){
        host->productName.chars[0] = (char) toupper(host->productName.chars[0]);
        ffStrbufAppendC(&host->productName, ' ');
    }

    ffSettingsGetAndroidProperty("ro.product.model", &host->productName);

    ffStrbufTrimRight(&host->productName, ' ');

    //Sys vendor

    ffStrbufInit(&host->sysVendor);
    ffSettingsGetAndroidProperty("ro.product.manufacturer", &host->sysVendor);

    //Not implemented

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
}
