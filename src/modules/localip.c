#include "fastfetch.h"
#include "common/printing.h"
#include "detection/localip/localip.h"

#define FF_LOCALIP_MODULE_NAME "Local IP"
#define FF_LOCALIP_NUM_FORMAT_ARGS 2

static int sortIpsV4First(const FFLocalIpResult* left, const FFLocalIpResult* right)
{
    if (left->ipv6 != right->ipv6)
        return left->ipv6 == false ? -1 : 1;

    return ffStrbufComp(&left->addr, &right->addr);
}

static int sortIpsV6First(const FFLocalIpResult* left, const FFLocalIpResult* right)
{
    if (left->ipv6 != right->ipv6)
        return left->ipv6 == true ? -1 : 1;

    return ffStrbufComp(&left->addr, &right->addr);
}

void ffPrintLocalIp(FFinstance* instance)
{
    FF_LIST_AUTO_DESTROY results;
    ffListInit(&results, sizeof(FFLocalIpResult));

    const char* error = ffDetectLocalIps(instance, &results);

    if(error)
    {
        ffPrintError(instance, FF_LOCALIP_MODULE_NAME, 0, &instance->config.localIP, "%s", error);
        return;
    }

    if(results.length == 0)
    {
        ffPrintError(instance, FF_LOCALIP_MODULE_NAME, 0, &instance->config.localIP, "Failed to detect any IPs");
        return;
    }

    if (instance->config.localIpCompactType != FF_LOCALIP_COMPACT_TYPE_NONE)
    {
        ffPrintLogoAndKey(instance, FF_LOCALIP_MODULE_NAME, 0, &instance->config.localIP.key);

        switch (instance->config.localIpCompactType)
        {
            case FF_LOCALIP_COMPACT_TYPE_V4FIRST:
                ffListSort(&results, (void*) sortIpsV4First);
                break;
            case FF_LOCALIP_COMPACT_TYPE_V6FIRST:
                ffListSort(&results, (void*) sortIpsV6First);
                break;
            default:
                break;
        }

        uint32_t index = 0;
        FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
        {
            if (index++ > 0)
                putchar(' ');
            ffStrbufWriteTo(&ip->addr, stdout);
        }
        putchar('\n');
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY key;
        ffStrbufInit(&key);

        FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
        {
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
    }
}
