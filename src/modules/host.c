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
    if(instance->config.hostShowVersion || instance->config.hostFormat[0] != '\0')
        ffGetFileContent("/sys/devices/virtual/dmi/id/product_version", version, sizeof(version));
    else
        version[0] = '\0';

    char host[1024];

    if(instance->config.hostFormat[0] != '\0')
    {
        snprintf(host, sizeof(host), instance->config.hostFormat, family, name, version);
    }
    else if(family[0] == '\0' && name[0] == '\0')
    {
        ffPrintError(instance, "Host", "neither family nor name could be determined");
        return;
    }else if(family[0] != '\0' && name[0] != '\0')
    {
        snprintf(host, sizeof(host), "%s %s %s", family, name, instance->config.hostShowVersion ? version : "");
    }
    else if(family[0] != '\0')
    {
        snprintf(host, sizeof(host), "%s %s", family, instance->config.hostShowVersion ? version : "");
    }
    else
    {
        snprintf(host, sizeof(host), "%s %s", name, instance->config.hostShowVersion ? version : "");
    }

    ffPrintAndSaveCachedValue(instance, "Host", host);
}