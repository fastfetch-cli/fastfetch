#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/localip/localip.h"
#include "modules/localip/localip.h"
#include "util/stringUtils.h"

#define FF_LOCALIP_DISPLAY_NAME "Local IP"
#define FF_LOCALIP_NUM_FORMAT_ARGS 5
#pragma GCC diagnostic ignored "-Wsign-conversion"

static int sortIps(const FFLocalIpResult* left, const FFLocalIpResult* right)
{
    return ffStrbufComp(&left->name, &right->name);
}

static void formatKey(const FFLocalIpOptions* options, FFLocalIpResult* ip, uint32_t index, FFstrbuf* key)
{
    if(options->moduleArgs.key.length == 0)
    {
        if(!ip->name.length)
            ffStrbufSetF(&ip->name, "unknown %u", (unsigned) index);

        ffStrbufSetF(key, FF_LOCALIP_DISPLAY_NAME " (%s)", ip->name.chars);
    }
    else
    {
        ffStrbufClear(key);
        FF_PARSE_FORMAT_STRING_CHECKED(key, &options->moduleArgs.key, 3, ((FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &index},
            {FF_FORMAT_ARG_TYPE_STRBUF, &ip->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &ip->mac},
        }));
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
        ffPrintError(FF_LOCALIP_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(results.length == 0)
    {
        ffPrintError(FF_LOCALIP_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Failed to detect any IPs");
        return;
    }

    ffListSort(&results, (const void*) sortIps);

    if (options->showType & FF_LOCALIP_TYPE_COMPACT_BIT)
    {
        ffPrintLogoAndKey(FF_LOCALIP_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        bool flag = false;

        FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
        {
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
        uint32_t index = 0;

        FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
        {
            formatKey(options, ip, results.length == 1 ? 0 : index + 1, &key);
            if(options->moduleArgs.outputFormat.length == 0)
            {
                ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);
                printIp(ip, true);
                putchar('\n');
            }
            else
            {
                FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_LOCALIP_NUM_FORMAT_ARGS, ((FFformatarg[]){
                    {FF_FORMAT_ARG_TYPE_STRBUF, &ip->ipv4},
                    {FF_FORMAT_ARG_TYPE_STRBUF, &ip->ipv6},
                    {FF_FORMAT_ARG_TYPE_STRBUF, &ip->mac},
                    {FF_FORMAT_ARG_TYPE_STRBUF, &ip->name},
                    {FF_FORMAT_ARG_TYPE_BOOL, &ip->defaultRoute},
                }));
            }
            ++index;
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

    if (ffStrEqualsIgnCase(subKey, "show-prefix-len"))
    {
        if (ffOptionParseBoolean(value))
            options->showType |= FF_LOCALIP_TYPE_PREFIX_LEN_BIT;
        else
            options->showType &= ~FF_LOCALIP_TYPE_PREFIX_LEN_BIT;
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

void ffParseLocalIpJsonObject(FFLocalIpOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffStrEqualsIgnCase(key, "showIpv4"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_IPV4_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_IPV4_BIT;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "showIpv6"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_IPV6_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_IPV6_BIT;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "showMac"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_MAC_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_MAC_BIT;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "showLoop"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_LOOP_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_LOOP_BIT;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "showPrefixLen"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_PREFIX_LEN_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_PREFIX_LEN_BIT;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "compact"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_COMPACT_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_COMPACT_BIT;
            continue;
        }

        if (ffStrEqualsIgnCase(key, "namePrefix"))
        {
            ffStrbufSetS(&options->namePrefix, yyjson_get_str(val));
            continue;
        }

        if (ffStrEqualsIgnCase(key, "defaultRouteOnly"))
        {
            options->defaultRouteOnly = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_LOCALIP_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateLocalIpJsonConfig(FFLocalIpOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyLocalIpOptions))) FFLocalIpOptions defaultOptions;
    ffInitLocalIpOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (defaultOptions.showType != options->showType)
    {
        if (options->showType & FF_LOCALIP_TYPE_IPV4_BIT)
            yyjson_mut_obj_add_bool(doc, module, "showIpv4", true);

        if (options->showType & FF_LOCALIP_TYPE_IPV6_BIT)
            yyjson_mut_obj_add_bool(doc, module, "showIpv6", true);

        if (options->showType & FF_LOCALIP_TYPE_MAC_BIT)
            yyjson_mut_obj_add_bool(doc, module, "showMac", true);

        if (options->showType & FF_LOCALIP_TYPE_LOOP_BIT)
            yyjson_mut_obj_add_bool(doc, module, "showLoop", true);

        if (options->showType & FF_LOCALIP_TYPE_PREFIX_LEN_BIT)
            yyjson_mut_obj_add_bool(doc, module, "showPrefixLen", true);

        if (options->showType & FF_LOCALIP_TYPE_COMPACT_BIT)
            yyjson_mut_obj_add_bool(doc, module, "compact", true);
    }

    if (!ffStrbufEqual(&options->namePrefix, &defaultOptions.namePrefix))
        yyjson_mut_obj_add_strbuf(doc, module, "namePrefix", &options->namePrefix);

    if (options->defaultRouteOnly != defaultOptions.defaultRouteOnly)
        yyjson_mut_obj_add_bool(doc, module, "defaultRouteOnly", options->defaultRouteOnly);
}

void ffGenerateLocalIpJsonResult(FF_MAYBE_UNUSED FFLocalIpOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFLocalIpResult));

    const char* error = ffDetectLocalIps(options, &results);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_bool(doc, obj, "defaultRoute", ip->defaultRoute);
        yyjson_mut_obj_add_strbuf(doc, obj, "ipv4", &ip->ipv4);
        yyjson_mut_obj_add_strbuf(doc, obj, "ipv6", &ip->ipv6);
        yyjson_mut_obj_add_strbuf(doc, obj, "mac", &ip->mac);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &ip->name);
    }

    FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
    {
        ffStrbufDestroy(&ip->name);
        ffStrbufDestroy(&ip->ipv4);
        ffStrbufDestroy(&ip->ipv6);
        ffStrbufDestroy(&ip->mac);
    }
}

void ffPrintLocalIpHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_LOCALIP_MODULE_NAME, "{1}", FF_LOCALIP_NUM_FORMAT_ARGS, ((const char* []) {
        "Local IPv4 address",
        "Local IPv6 address",
        "Physical (MAC) address",
        "Interface name",
        "Is default route"
    }));
}

void ffInitLocalIpOptions(FFLocalIpOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_LOCALIP_MODULE_NAME,
        "List local IP addresses (v4 or v6), MAC addresses, etc",
        ffParseLocalIpCommandOptions,
        ffParseLocalIpJsonObject,
        ffPrintLocalIp,
        ffGenerateLocalIpJsonResult,
        ffPrintLocalIpHelpFormat,
        ffGenerateLocalIpJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);

    options->showType = FF_LOCALIP_TYPE_IPV4_BIT | FF_LOCALIP_TYPE_PREFIX_LEN_BIT;
    ffStrbufInit(&options->namePrefix);
    options->defaultRouteOnly =
        #ifdef __ANDROID__
            false
        #else
            true
        #endif
    ;
}

void ffDestroyLocalIpOptions(FFLocalIpOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->namePrefix);
}
