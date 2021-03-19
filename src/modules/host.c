#include "fastfetch.h"

void ffPrintHost(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, "Host"))
        return;

    char family[256];
    ffGetFileContent("/sys/devices/virtual/dmi/id/product_family", family, sizeof(family));

    char name[256];
    ffGetFileContent("/sys/devices/virtual/dmi/id/product_name", name, sizeof(name));
    
    char version[256];
    ffGetFileContent("/sys/devices/virtual/dmi/id/product_version", version, sizeof(version));

    if(family[0] == '\0' && name[0] == '\0')
    {
        ffPrintError(instance, "Host", "neither family nor name could be determined");
        return;
    }

    ffPrintLogoAndKey(instance, "Host");

    FFstrbuf host;
    ffStrbufInit(&host);

    if(!ffStrbufIsEmpty(&instance->config.hostFormat))
    {
        ffParseFormatString(&host, &instance->config.hostFormat, 3,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, family},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, name},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, version}
        );
    }
    else if(family[0] != '\0' && name[0] != '\0')
    {
        ffStrbufSetF(&host, "%s %s %s", family, name, version);
    }
    else if(family[0] != '\0')
    {
        ffStrbufSetF(&host, "%s %s", family, version);
    }
    else
    {
        ffStrbufSetF(&host, "%s %s", name, version);
    }

    ffPrintAndSaveCachedValue(instance, "Host", host.chars);
    ffStrbufDestroy(&host);
}