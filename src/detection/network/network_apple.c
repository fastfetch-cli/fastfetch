#include "detection/network/network.h"
#include "util/apple/cf_helpers.h"

#include <SystemConfiguration/SystemConfiguration.h>

const char* ffDetectNetwork(FFinstance* instance, FFlist* result)
{
    FF_STRBUF_AUTO_DESTROY sbType;
    ffStrbufInit(&sbType);

    CFArrayRef FF_CFTYPE_AUTO_RELEASE array = SCNetworkInterfaceCopyAll();
    for (CFIndex i = 0, length = CFArrayGetCount(array); i < length; ++i)
    {
        SCNetworkInterfaceRef ni = CFArrayGetValueAtIndex(array, i), parent;
        while ((parent = SCNetworkInterfaceGetInterface(ni)))
            ni = parent;

        CFStringRef cfType = SCNetworkInterfaceGetInterfaceType(ni);
        if (cfType)
            ffCfStrGetString(cfType, &sbType);
        else
            ffStrbufAppendS(&sbType, "Unknown");

        if (instance->config.networkType.length && !ffStrbufContainIgnCase(&instance->config.networkType, &sbType))
            continue;

        FFNetworkResult* item = (FFNetworkResult*) ffListAdd(result);
        ffStrbufInitMove(&item->type, &sbType);
        ffStrbufInit(&item->name);
        ffStrbufInit(&item->address);
        item->mtu = 0;

        ffCfStrGetString(SCNetworkInterfaceGetLocalizedDisplayName(ni), &item->name);
        ffCfStrGetString(SCNetworkInterfaceGetHardwareAddressString(ni), &item->address);
        SCNetworkInterfaceCopyMTU(ni, &item->mtu, nil, nil);
    }

    if (result->length == 0)
        return "No network interfaces found";

    return NULL;
}
