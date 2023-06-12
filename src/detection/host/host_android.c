#include "host.h"
#include "common/settings.h"
#include <ctype.h>

const char* ffDetectHost(FFHostResult* host)
{
    //Family

    ffStrbufInit(&host->productFamily);
    ffSettingsGetAndroidProperty("ro.product.device", &host->productFamily);

    //Name

    ffStrbufInit(&host->productName);

    ffSettingsGetAndroidProperty("ro.product.brand", &host->productName);
    if(host->productName.length > 0)
    {
        host->productName.chars[0] = (char) toupper(host->productName.chars[0]);
        ffStrbufAppendC(&host->productName, ' ');
    }

    ffSettingsGetAndroidProperty("ro.product.model", &host->productName);

    ffStrbufTrimRight(&host->productName, ' ');

    //Sys vendor

    ffStrbufInit(&host->sysVendor);
    ffSettingsGetAndroidProperty("ro.product.manufacturer", &host->sysVendor);

    //Not implemented

    ffStrbufInit(&host->productVersion);
    ffStrbufInit(&host->productSku);

    return NULL;
}
