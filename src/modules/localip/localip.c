#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/localip/localip.h"
#include "modules/localip/localip.h"
#include "util/stringUtils.h"

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

static void printIp(FFLocalIpResult* ip, bool markDefaultRoute)
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
    if (markDefaultRoute && flag && ip->defaultRoute)
        fputs(" *", stdout);
}

void ffPrintLocalIp(FFLocalIpOptions* options)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFLocalIpResult));

    const char* error = ffDetectLocalIps(options, &results);

    if(error)
    {
        ffPrintError(FF_LOCALIP_DISPLAY_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(results.length == 0)
    {
        ffPrintError(FF_LOCALIP_DISPLAY_NAME, 0, &options->moduleArgs, "Failed to detect any IPs");
        return;
    }

    ffListSort(&results, (const void*) sortIps);

    if (options->showType & FF_LOCALIP_TYPE_COMPACT_BIT)
    {
        ffPrintLogoAndKey(FF_LOCALIP_DISPLAY_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);

        bool flag = false;

        FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
        {
            if (options->defaultRouteOnly && !ip->defaultRoute)
                continue;

            if (flag)
                fputs(" - ", stdout);
            else
                flag = true;
            printIp(ip, false);
        }
        putchar('\n');
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

        FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
        {
            if (options->defaultRouteOnly && !ip->defaultRoute)
                continue;

            formatKey(options, ip, &key);
            if(options->moduleArgs.outputFormat.length == 0)
            {
                ffPrintLogoAndKey(key.chars, 0, NULL, &options->moduleArgs.keyColor);
                printIp(ip, !options->defaultRouteOnly);
                putchar('\n');
            }
            else
            {
                ffPrintFormatString(key.chars, 0, NULL, &options->moduleArgs.keyColor, &options->moduleArgs.outputFormat, FF_LOCALIP_NUM_FORMAT_ARGS, (FFformatarg[]){
                    {FF_FORMAT_ARG_TYPE_STRBUF, &ip->ipv4},
                    {FF_FORMAT_ARG_TYPE_STRBUF, &ip->ipv6},
                    {FF_FORMAT_ARG_TYPE_STRBUF, &ip->mac},
                    {FF_FORMAT_ARG_TYPE_STRBUF, &ip->name},
                    {FF_FORMAT_ARG_TYPE_BOOL, &ip->defaultRoute},
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
    options->defaultRouteOnly = false;
}

bool ffParseLocalIpCommandOptions(FFLocalIpOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_LOCALIP_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "show-ipv4"))
    {
        if (ffOptionParseBoolean(value))
            options->showType |= FF_LOCALIP_TYPE_IPV4_BIT;
        else
            options->showType &= ~FF_LOCALIP_TYPE_IPV4_BIT;
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "show-ipv6"))
    {
        if (ffOptionParseBoolean(value))
            options->showType |= FF_LOCALIP_TYPE_IPV6_BIT;
        else
            options->showType &= ~FF_LOCALIP_TYPE_IPV6_BIT;
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "show-mac"))
    {
        if (ffOptionParseBoolean(value))
            options->showType |= FF_LOCALIP_TYPE_MAC_BIT;
        else
            options->showType &= ~FF_LOCALIP_TYPE_MAC_BIT;
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "show-loop"))
    {
        if (ffOptionParseBoolean(value))
            options->showType |= FF_LOCALIP_TYPE_LOOP_BIT;
        else
            options->showType &= ~FF_LOCALIP_TYPE_LOOP_BIT;
        return true;
    }

    if(ffStrEqualsIgnCase(subKey, "compact"))
    {
        if (ffOptionParseBoolean(value))
            options->showType |= FF_LOCALIP_TYPE_COMPACT_BIT;
        else
            options->showType &= ~FF_LOCALIP_TYPE_COMPACT_BIT;
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "name-prefix"))
    {
        ffOptionParseString(key, value, &options->namePrefix);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "default-route-only"))
    {
        options->defaultRouteOnly = ffOptionParseBoolean(value);
        return true;
    }

    return false;
}

void ffDestroyLocalIpOptions(FFLocalIpOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->namePrefix);
}

void ffParseLocalIpJsonObject(yyjson_val* module)
{
    FFLocalIpOptions __attribute__((__cleanup__(ffDestroyLocalIpOptions))) options;
    ffInitLocalIpOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (ffStrEqualsIgnCase(key, "showIpv4"))
            {
                if (yyjson_get_bool(val))
                    options.showType |= FF_LOCALIP_TYPE_IPV4_BIT;
                else
                    options.showType &= ~FF_LOCALIP_TYPE_IPV4_BIT;
                continue;
            }

            if (ffStrEqualsIgnCase(key, "showIpv6"))
            {
                if (yyjson_get_bool(val))
                    options.showType |= FF_LOCALIP_TYPE_IPV6_BIT;
                else
                    options.showType &= ~FF_LOCALIP_TYPE_IPV6_BIT;
                continue;
            }

            if (ffStrEqualsIgnCase(key, "showMac"))
            {
                if (yyjson_get_bool(val))
                    options.showType |= FF_LOCALIP_TYPE_MAC_BIT;
                else
                    options.showType &= ~FF_LOCALIP_TYPE_MAC_BIT;
                continue;
            }

            if (ffStrEqualsIgnCase(key, "showLoop"))
            {
                if (yyjson_get_bool(val))
                    options.showType |= FF_LOCALIP_TYPE_LOOP_BIT;
                else
                    options.showType &= ~FF_LOCALIP_TYPE_LOOP_BIT;
                continue;
            }

            if (ffStrEqualsIgnCase(key, "compact"))
            {
                if (yyjson_get_bool(val))
                    options.showType |= FF_LOCALIP_TYPE_COMPACT_BIT;
                else
                    options.showType &= ~FF_LOCALIP_TYPE_COMPACT_BIT;
                continue;
            }

            if (ffStrEqualsIgnCase(key, "namePrefix"))
            {
                ffStrbufSetS(&options.namePrefix, yyjson_get_str(val));
                continue;
            }

            if (ffStrEqualsIgnCase(key, "defaultRouteOnly"))
            {
                options.defaultRouteOnly = yyjson_get_bool(val);
                continue;
            }

            ffPrintError(FF_LOCALIP_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintLocalIp(&options);
}
