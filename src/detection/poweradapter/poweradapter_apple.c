#include "fastfetch.h"
#include "poweradapter.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>

const char* ffDetectPowerAdapter(FFlist* results)
{
    FF_CFTYPE_AUTO_RELEASE CFDictionaryRef details = IOPSCopyExternalPowerAdapterDetails();
    if (details && CFDictionaryContainsKey(details, CFSTR(kIOPSPowerAdapterWattsKey)))
    {
        FFPowerAdapterResult* adapter = ffListAdd(results);

        ffStrbufInit(&adapter->name);
        ffStrbufInit(&adapter->description);
        ffStrbufInit(&adapter->manufacturer);
        ffStrbufInit(&adapter->modelName);
        ffStrbufInit(&adapter->serial);
        adapter->watts = 0;

        ffCfDictGetString(details, CFSTR(kIOPSNameKey), &adapter->name);
        if (ffCfDictGetString(details, CFSTR("Model"), &adapter->modelName) != NULL)
        {
            int adapterId;
            if (ffCfDictGetInt(details, CFSTR(kIOPSPowerAdapterIDKey), &adapterId) == 0)
                ffStrbufSetF(&adapter->modelName, "%d", adapterId);
        }
        ffCfDictGetString(details, CFSTR("Manufacturer"), &adapter->manufacturer);
        ffCfDictGetString(details, CFSTR("Description"), &adapter->description);
        if (ffCfDictGetString(details, CFSTR("SerialString"), &adapter->serial) != NULL)
        {
            int serialNumber;
            if (ffCfDictGetInt(details, CFSTR(kIOPSPowerAdapterSerialNumberKey), &serialNumber) == 0)
                ffStrbufSetF(&adapter->serial, "%X", serialNumber);
        }
        ffCfDictGetInt(details, CFSTR(kIOPSPowerAdapterWattsKey), &adapter->watts);
    }

    return NULL;
}
