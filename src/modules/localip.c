#include "fastfetch.h"
#include "common/printing.h"
#include "detection/localip/localip.h"

#define FF_LOCALIP_MODULE_NAME "Local IP"
#define FF_LOCALIP_NUM_FORMAT_ARGS 2

void ffPrintLocalIp(FFinstance* instance)
{
    FFlist results;
    ffListInit(&results, sizeof(FFLocalIpResult));

    const char* error = ffDetectLocalIps(instance, &results);

    if(error)
    {
        ffPrintError(instance, FF_LOCALIP_MODULE_NAME, 0, &instance->config.localIP, "%s", error);
        goto exit;
    }

    if(results.length == 0)
    {
        ffPrintError(instance, FF_LOCALIP_MODULE_NAME, 0, &instance->config.localIP, "Failed to detect any IPs");
        goto exit;
    }

    FFstrbuf key;
    ffStrbufInit(&key);

    for(uint32_t i = 0; i < results.length; ++i)
    {
        FFLocalIpResult* ip = (FFLocalIpResult*) ffListGet(&results, i);

        if(instance->config.localIP.key.length == 0)
        {
            if(ip->name.length)
                ffStrbufSetF(&key, FF_LOCALIP_MODULE_NAME " (%*s)", ip->name.length, ip->name.chars);
            else
                ffStrbufSetS(&key, FF_LOCALIP_MODULE_NAME);
        }
        else
        {
            ffStrbufClear(&key);
            ffParseFormatString(&key, &instance->config.localIP.key, 1, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &ip->name}
            });
        }

        if(instance->config.localIP.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, key.chars, 0, NULL);
            ffStrbufPutTo(&ip->addr, stdout);
        }
        else
        {
            ffPrintFormatString(instance, key.chars, 0, NULL, &instance->config.localIP.outputFormat, FF_LOCALIP_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &ip->addr},
                {FF_FORMAT_ARG_TYPE_STRING, ip->ipv6 ? "IPv6" : "IPv4"}
            });
        }

        ffStrbufDestroy(&ip->name);
        ffStrbufDestroy(&ip->addr);
    }

    ffStrbufDestroy(&key);

exit:
    ffListDestroy(&results);
}
