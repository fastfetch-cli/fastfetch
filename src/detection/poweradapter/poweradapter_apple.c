#include "fastfetch.h"
#include "poweradapter.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>

const char* ffDetectPowerAdapter(FFlist* results)
{
    FF_CFTYPE_AUTO_RELEASE CFDictionaryRef details = IOPSCopyExternalPowerAdapterDetails();
    FFPowerAdapterResult* adapter = ffListAdd(results);

    ffStrbufInit(&adapter->name);
    ffStrbufInit(&adapter->description);
    ffStrbufInit(&adapter->manufacturer);
    ffStrbufInit(&adapter->modelName);
    ffStrbufInit(&adapter->serial);
    adapter->watts = FF_POWERADAPTER_NOT_CONNECTED;

    if (details)
    {
        ffCfDictGetString(details, CFSTR(kIOPSNameKey), &adapter->name);
        ffCfDictGetString(details, CFSTR("Model"), &adapter->modelName);
        ffCfDictGetString(details, CFSTR("Manufacturer"), &adapter->manufacturer);
        ffCfDictGetString(details, CFSTR("Description"), &adapter->description);
        ffCfDictGetString(details, CFSTR("SerialString"), &adapter->serial);
        ffCfDictGetInt(details, CFSTR(kIOPSPowerAdapterWattsKey), &adapter->watts);
    }

    return NULL;
}
