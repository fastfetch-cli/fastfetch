#include "fastfetch.h"

void ffPrintHost(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, "Host"))
        return;

    char host[256];
    ffGetFileContent("/sys/devices/virtual/dmi/id/product_name", host, sizeof(host) - 32); //We subtract 32 to have at least this padding for the version
    if(host[0] == '\0')
    {
        ffPrintError(instance, "Host", "ffGetFileContent(\"/sys/devices/virtual/dmi/id/product_name\", host, sizeof(host)) failed");
        return;
    }

    if(instance->config.hostShowVersion)
    {
        size_t len = strlen(host);
        host[len++] = ' ';
        ffGetFileContent("/sys/devices/virtual/dmi/id/product_version", host + len, sizeof(host) - len);
    }

    ffPrintAndSaveCachedValue(instance, "Host", host);
}