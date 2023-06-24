#include "host.h"
#include "common/settings.h"
#include <ctype.h>

const char* ffDetectHost(FFHostResult* host)
{
    ffSettingsGetAndroidProperty("ro.product.device", &host->productFamily);

    ffSettingsGetAndroidProperty("ro.product.brand", &host->productName);
    if(host->productName.length > 0)
    {
        host->productName.chars[0] = (char) toupper(host->productName.chars[0]);
        ffStrbufAppendC(&host->productName, ' ');
    }

    ffSettingsGetAndroidProperty("ro.product.model", &host->productName);
    ffStrbufTrimRight(&host->productName, ' ');

    ffSettingsGetAndroidProperty("ro.product.manufacturer", &host->sysVendor);

    return NULL;
}
