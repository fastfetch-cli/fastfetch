#include "fastfetch.h"
#include "common/printing.h"
#include "detection/localip/localip.h"

#define FF_LOCALIP_MODULE_NAME "Local IP"
#define FF_LOCALIP_NUM_FORMAT_ARGS 2

static int sortIps(const FFLocalIpResult* left, const FFLocalIpResult* right)
{
    return ffStrbufComp(&left->name, &right->name);
}

static void formatKey(const FFinstance* instance, const FFLocalIpResult* ip, FFstrbuf* key)
{
    if(instance->config.localIP.key.length == 0)
    {
        if(ip->name.length)
            ffStrbufSetF(key, FF_LOCALIP_MODULE_NAME " (%*s)", ip->name.length, ip->name.chars);
        else
            ffStrbufSetS(key, FF_LOCALIP_MODULE_NAME);
    }
    else
    {
        ffStrbufClear(key);
        ffParseFormatString(key, &instance->config.localIP.key, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &ip->name}
        });
    }
}

static void printIp(FFLocalIpResult* ip)
{
    bool flag = false;
    if (ip->ipv4.length)
    {
        ffStrbufWriteTo(&ip->ipv4, stdout);
        flag = true;
    }
    if (ip->ipv6.length)
    {
        if (flag) putchar(' ');
        ffStrbufWriteTo(&ip->ipv6, stdout);
        flag = true;
    }
    if (ip->mac.length)
    {
        if (flag)
            printf(" (%s)", ip->mac.chars);
        else
            ffStrbufWriteTo(&ip->mac, stdout);
    }
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

    ffListSort(&results, (void*) sortIps);

    if (instance->config.localIpShowType & FF_LOCALIP_TYPE_COMPACT_BIT)
    {
        ffPrintLogoAndKey(instance, FF_LOCALIP_MODULE_NAME, 0, &instance->config.localIP.key);

        FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
        {
            if ((void*) ip != (void*) results.data)
                fputs(" - ", stdout);
            printIp(ip);
        }
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY key;
        ffStrbufInit(&key);

        FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
        {
            formatKey(instance, ip, &key);
            if(instance->config.localIP.outputFormat.length == 0)
            {
                ffPrintLogoAndKey(instance, key.chars, 0, NULL);
                printIp(ip);
                putchar('\n');
            }
            else
            {
                ffPrintFormatString(instance, key.chars, 0, NULL, &instance->config.localIP.outputFormat, FF_LOCALIP_NUM_FORMAT_ARGS, (FFformatarg[]){
                    {FF_FORMAT_ARG_TYPE_STRBUF, &ip->ipv4},
                    {FF_FORMAT_ARG_TYPE_STRBUF, &ip->ipv6},
                    {FF_FORMAT_ARG_TYPE_STRBUF, &ip->mac},
                    {FF_FORMAT_ARG_TYPE_STRBUF, &ip->name},
                });
            }
        }
    }

    FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
    {
        ffStrbufDestroy(&ip->name);
        ffStrbufDestroy(&ip->ipv4);
        ffStrbufDestroy(&ip->ipv6);
        ffStrbufDestroy(&ip->mac);
    }
}
