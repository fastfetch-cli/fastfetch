#include "fastfetch.h"
#include "common/printing.h"
#include "detection/localip/localip.h"
#include "modules/localip/localip.h"

#define FF_LOCALIP_DISPLAY_NAME "Local IP"
#define FF_LOCALIP_NUM_FORMAT_ARGS 2
#pragma GCC diagnostic ignored "-Wsign-conversion"

static int sortIps(const FFLocalIpResult* left, const FFLocalIpResult* right)
{
    return ffStrbufComp(&left->name, &right->name);
}

static void formatKey(const FFLocalIpOptions* options, const FFLocalIpResult* ip, FFstrbuf* key)
{
    if(options->moduleArgs.key.length == 0)
    {
        if(ip->name.length)
            ffStrbufSetF(key, FF_LOCALIP_DISPLAY_NAME " (%s)", ip->name.chars);
        else
            ffStrbufSetS(key, FF_LOCALIP_DISPLAY_NAME);
    }
    else
    {
        ffStrbufClear(key);
        ffParseFormatString(key, &options->moduleArgs.key, 2, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &ip->name},
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

void ffPrintLocalIp(FFinstance* instance, FFLocalIpOptions* options)
{
    FF_LIST_AUTO_DESTROY results;
    ffListInit(&results, sizeof(FFLocalIpResult));

    const char* error = ffDetectLocalIps(options, &results);

    if(error)
    {
        ffPrintError(instance, FF_LOCALIP_DISPLAY_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(results.length == 0)
    {
        ffPrintError(instance, FF_LOCALIP_DISPLAY_NAME, 0, &options->moduleArgs, "Failed to detect any IPs");
        return;
    }

    ffListSort(&results, (const void*) sortIps);

    if (options->showType & FF_LOCALIP_TYPE_COMPACT_BIT)
    {
        ffPrintLogoAndKey(instance, FF_LOCALIP_DISPLAY_NAME, 0, &options->moduleArgs.key);

        FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
        {
            if ((void*) ip != (void*) results.data)
                fputs(" - ", stdout);
            printIp(ip);
        }
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

        FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
        {
            formatKey(options, ip, &key);
            if(options->moduleArgs.outputFormat.length == 0)
            {
                ffPrintLogoAndKey(instance, key.chars, 0, NULL);
                printIp(ip);
                putchar('\n');
            }
            else
            {
                ffPrintFormatString(instance, key.chars, 0, NULL, &options->moduleArgs.outputFormat, FF_LOCALIP_NUM_FORMAT_ARGS, (FFformatarg[]){
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

void ffInitLocalIpOptions(FFLocalIpOptions* options)
{
    options->moduleName = FF_LOCALIP_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);

    options->showType = FF_LOCALIP_TYPE_IPV4_BIT;
    ffStrbufInit(&options->namePrefix);
}

bool ffParseLocalIpCommandOptions(FFLocalIpOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_LOCALIP_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (strcasecmp(subKey, "show-ipv4") == 0)
    {
        if (ffOptionParseBoolean(value))
            options->showType |= FF_LOCALIP_TYPE_IPV4_BIT;
        else
            options->showType &= ~FF_LOCALIP_TYPE_IPV4_BIT;
        return true;
    }

    if (strcasecmp(subKey, "show-ipv6") == 0)
    {
        if (ffOptionParseBoolean(value))
            options->showType |= FF_LOCALIP_TYPE_IPV6_BIT;
        else
            options->showType &= ~FF_LOCALIP_TYPE_IPV6_BIT;
        return true;
    }

    if (strcasecmp(subKey, "show-mac") == 0)
    {
        if (ffOptionParseBoolean(value))
            options->showType |= FF_LOCALIP_TYPE_MAC_BIT;
        else
            options->showType &= ~FF_LOCALIP_TYPE_MAC_BIT;
        return true;
    }

    if (strcasecmp(subKey, "show-loop") == 0)
    {
        if (ffOptionParseBoolean(value))
            options->showType |= FF_LOCALIP_TYPE_LOOP_BIT;
        else
            options->showType &= ~FF_LOCALIP_TYPE_LOOP_BIT;
        return true;
    }

    if(strcasecmp(subKey, "compact") == 0)
    {
        if (ffOptionParseBoolean(value))
            options->showType |= FF_LOCALIP_TYPE_COMPACT_BIT;
        else
            options->showType &= ~FF_LOCALIP_TYPE_COMPACT_BIT;
        return true;
    }

    if (strcasecmp(subKey, "name-prefix") == 0)
    {
        ffOptionParseString(key, value, &options->namePrefix);
        return true;
    }

    return false;
}

void ffDestroyLocalIpOptions(FFLocalIpOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->namePrefix);
}

#ifdef FF_HAVE_JSONC
void ffParseLocalIpJsonObject(FFinstance* instance, json_object* module)
{
    FFLocalIpOptions __attribute__((__cleanup__(ffDestroyLocalIpOptions))) options;
    ffInitLocalIpOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (strcasecmp(key, "showIpv4") == 0)
            {
                if (json_object_get_boolean(val))
                    options.showType |= FF_LOCALIP_TYPE_IPV4_BIT;
                else
                    options.showType &= ~FF_LOCALIP_TYPE_IPV4_BIT;
                continue;
            }

            if (strcasecmp(key, "showIpv6") == 0)
            {
                if (json_object_get_boolean(val))
                    options.showType |= FF_LOCALIP_TYPE_IPV6_BIT;
                else
                    options.showType &= ~FF_LOCALIP_TYPE_IPV6_BIT;
                continue;
            }

            if (strcasecmp(key, "showMac") == 0)
            {
                if (json_object_get_boolean(val))
                    options.showType |= FF_LOCALIP_TYPE_MAC_BIT;
                else
                    options.showType &= ~FF_LOCALIP_TYPE_MAC_BIT;
                continue;
            }

            if (strcasecmp(key, "showLoop") == 0)
            {
                if (json_object_get_boolean(val))
                    options.showType |= FF_LOCALIP_TYPE_LOOP_BIT;
                else
                    options.showType &= ~FF_LOCALIP_TYPE_LOOP_BIT;
                continue;
            }

            if (strcasecmp(key, "compact") == 0)
            {
                if (json_object_get_boolean(val))
                    options.showType |= FF_LOCALIP_TYPE_COMPACT_BIT;
                else
                    options.showType &= ~FF_LOCALIP_TYPE_COMPACT_BIT;
                continue;
            }

            if (strcasecmp(key, "namePrefix") == 0)
            {
                ffStrbufSetS(&options.namePrefix, json_object_get_string(val));
                continue;
            }

            ffPrintError(instance, FF_LOCALIP_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintLocalIp(instance, &options);
}
#endif
