#include "host.h"
#include "common/settings.h"
#include <ctype.h>

const char* ffDetectHost(FFHostResult* host)
{
    // http://newandroidbook.com/ddb/
    ffSettingsGetAndroidProperty("ro.product.device", &host->family);

    ffSettingsGetAndroidProperty("ro.product.marketname", &host->name)
        || ffSettingsGetAndroidProperty("ro.vendor.product.display", &host->name)
        || ffSettingsGetAndroidProperty("ro.config.devicename", &host->name)
        || ffSettingsGetAndroidProperty("ro.config.marketing_name", &host->name)
        || ffSettingsGetAndroidProperty("ro.product.vendor.model", &host->name)
        || ffSettingsGetAndroidProperty("ro.product.oppo_model", &host->name)
        || ffSettingsGetAndroidProperty("ro.oppo.market.name", &host->name)
        || ffSettingsGetAndroidProperty("ro.product.brand", &host->name);

    if (ffSettingsGetAndroidProperty("ro.product.model", &host->version))
    {
        if (ffStrbufStartsWithIgnCase(&host->version, &host->name))
        {
            ffStrbufSubstrAfter(&host->version, host->name.length);
            ffStrbufTrimLeft(&host->version, ' ');
        }
    }

    ffSettingsGetAndroidProperty("ro.product.manufacturer", &host->vendor);

    if(host->vendor.length && !ffStrbufStartsWithIgnCase(&host->name, &host->vendor))
    {
        ffStrbufPrependS(&host->name, " ");
        ffStrbufPrepend(&host->name, &host->vendor);
    }

    return NULL;
}
