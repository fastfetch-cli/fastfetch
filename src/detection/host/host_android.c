#include "host.h"
#include "common/settings.h"
#include <ctype.h>

const char* ffDetectHost(FFHostResult* host)
{
    // http://newandroidbook.com/ddb/
    ffSettingsGetAndroidProperty("ro.product.device", &host->productFamily);

    ffSettingsGetAndroidProperty("ro.product.marketname", &host->productName)
        || ffSettingsGetAndroidProperty("ro.vendor.product.display", &host->productName)
        || ffSettingsGetAndroidProperty("ro.config.devicename", &host->productName)
        || ffSettingsGetAndroidProperty("ro.config.marketing_name", &host->productName)
        || ffSettingsGetAndroidProperty("ro.product.vendor.model", &host->productName)
        || ffSettingsGetAndroidProperty("ro.product.oppo_model", &host->productName)
        || ffSettingsGetAndroidProperty("ro.oppo.market.name", &host->productName)
        || ffSettingsGetAndroidProperty("ro.product.brand", &host->productName);

    if (ffSettingsGetAndroidProperty("ro.product.model", &host->productVersion))
    {
        if (ffStrbufStartsWithIgnCase(&host->productVersion, &host->productName))
        {
            ffStrbufSubstrAfter(&host->productVersion, host->productName.length);
            ffStrbufTrimLeft(&host->productVersion, ' ');
        }
    }

    ffSettingsGetAndroidProperty("ro.product.manufacturer", &host->sysVendor);

    if(host->sysVendor.length && !ffStrbufStartsWithIgnCase(&host->productName, &host->sysVendor))
    {
        ffStrbufPrependS(&host->productName, " ");
        ffStrbufPrepend(&host->productName, &host->sysVendor);
    }

    return NULL;
}
