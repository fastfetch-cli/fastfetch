#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/localip/localip.h"
#include "modules/localip/localip.h"
#include "util/stringUtils.h"

#define FF_LOCALIP_DISPLAY_NAME "Local IP"
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
        FF_PARSE_FORMAT_STRING_CHECKED(key, &options->moduleArgs.key, ((FFformatarg[]) {
            FF_FORMAT_ARG(index, "index"),
            FF_FORMAT_ARG(ip->name, "ifname"),
            FF_FORMAT_ARG(ip->mac, "mac"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }
}

static void appendSpeed(FFLocalIpResult* ip, FFstrbuf* strbuf)
{
    if (ip->speed >= 1000000)
    {
        ffStrbufAppendDouble(strbuf, ip->speed / 1e6, instance.config.display.fractionNdigits, instance.config.display.fractionTrailingZeros == FF_FRACTION_TRAILING_ZEROS_TYPE_ALWAYS);
        ffStrbufAppendS(strbuf, " Tbps");
    }
    else if (ip->speed >= 1000)
    {
        ffStrbufAppendDouble(strbuf, ip->speed / 1e3, instance.config.display.fractionNdigits, instance.config.display.fractionTrailingZeros == FF_FRACTION_TRAILING_ZEROS_TYPE_ALWAYS);
        ffStrbufAppendS(strbuf, " Gbps");
    }
    else
        ffStrbufAppendF(strbuf, "%u Mbps", (unsigned) ip->speed);
}

static void printIp(FFLocalIpResult* ip, bool markDefaultRoute, FFstrbuf* buffer)
{
    if (ip->ipv4.length)
    {
        ffStrbufAppend(buffer, &ip->ipv4);
    }
    if (ip->ipv6.length)
    {
        if (buffer->length) ffStrbufAppendC(buffer, ' ');
        ffStrbufAppend(buffer, &ip->ipv6);
    }
    if (ip->mac.length)
    {
        if (buffer->length)
            ffStrbufAppendF(buffer, " (%s)", ip->mac.chars);
        else
            ffStrbufAppend(buffer, &ip->mac);
    }
    if (ip->mtu > 0 || ip->speed > 0)
    {
        bool flag = buffer->length > 0;
        if (flag)
            ffStrbufAppendS(buffer, " [");
        if (ip->speed > 0)
        {
            if (ip->mtu > 0)
                ffStrbufAppendS(buffer, "Speed ");
            appendSpeed(ip, buffer);
            if (ip->mtu > 0)
                ffStrbufAppendS(buffer, " / MTU ");
        }
        if (ip->mtu > 0)
            ffStrbufAppendF(buffer, "%u", (unsigned) ip->mtu);
        if (flag)
            ffStrbufAppendC(buffer, ']');
    }
    if (ip->flags.length)
    {
        bool flag = buffer->length > 0;
        if (flag) ffStrbufAppendS(buffer, " <");
        ffStrbufAppend(buffer, &ip->flags);
        if (flag)
            ffStrbufAppendC(buffer, '>');
    }
    if (markDefaultRoute && ip->defaultRoute)
        ffStrbufAppendS(buffer, " *");
}

bool ffPrintLocalIp(FFLocalIpOptions* options)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFLocalIpResult));

    const char* error = ffDetectLocalIps(options, &results);

    if(error)
    {
        ffPrintError(FF_LOCALIP_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if(results.length == 0)
    {
        ffPrintError(FF_LOCALIP_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Failed to detect any IPs");
        return false;
    }

    ffListSort(&results, (const void*) sortIps);

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    if (options->showType & FF_LOCALIP_TYPE_COMPACT_BIT)
    {
        ffPrintLogoAndKey(FF_LOCALIP_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
        {
            if (buffer.length)
                ffStrbufAppendS(&buffer, " - ");
            printIp(ip, false, &buffer);
        }
        ffStrbufPutTo(&buffer, stdout);
        ffStrbufClear(&buffer);
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
                printIp(ip, !(options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT), &buffer);
                ffStrbufPutTo(&buffer, stdout);
            }
            else
            {
                if (ip->speed > 0)
                    appendSpeed(ip, &buffer);
                FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]){
                    FF_FORMAT_ARG(ip->ipv4, "ipv4"),
                    FF_FORMAT_ARG(ip->ipv6, "ipv6"),
                    FF_FORMAT_ARG(ip->mac, "mac"),
                    FF_FORMAT_ARG(ip->name, "ifname"),
                    FF_FORMAT_ARG(ip->defaultRoute, "is-default-route"),
                    FF_FORMAT_ARG(ip->mtu, "mtu"),
                    FF_FORMAT_ARG(buffer, "speed"),
                    FF_FORMAT_ARG(ip->flags, "flags"),
                }));
            }
            ++index;
            ffStrbufClear(&buffer);
        }
    }

    FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
    {
        ffStrbufDestroy(&ip->name);
        ffStrbufDestroy(&ip->ipv4);
        ffStrbufDestroy(&ip->ipv6);
        ffStrbufDestroy(&ip->mac);
        ffStrbufDestroy(&ip->flags);
    }

    return true;
}

void ffParseLocalIpJsonObject(FFLocalIpOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "showIpv4"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_IPV4_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_IPV4_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showIpv6"))
        {
            if (yyjson_is_bool(val))
            {
                options->ipv6Type = FF_LOCALIP_IPV6_TYPE_AUTO;
                if (unsafe_yyjson_get_bool(val))
                    options->showType |= FF_LOCALIP_TYPE_IPV6_BIT;
                else
                    options->showType &= ~FF_LOCALIP_TYPE_IPV6_BIT;
            }
            else if (yyjson_is_str(val))
            {
                int value;
                const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                    { "auto", FF_LOCALIP_IPV6_TYPE_AUTO },
                    { "gua", FF_LOCALIP_IPV6_TYPE_GUA_BIT },
                    { "ula", FF_LOCALIP_IPV6_TYPE_ULA_BIT },
                    { "lla", FF_LOCALIP_IPV6_TYPE_LLA_BIT },
                    { "unknown", FF_LOCALIP_IPV6_TYPE_UNKNOWN_BIT },
                    {},
                });
                if (error)
                    ffPrintError(FF_LOCALIP_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
                else
                {
                    options->showType |= FF_LOCALIP_TYPE_IPV6_BIT;
                    options->ipv6Type = (FFLocalIpIpv6Type) value;
                }
            }
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showMac"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_MAC_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_MAC_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showLoop"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_LOOP_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_LOOP_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showPrefixLen"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_PREFIX_LEN_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_PREFIX_LEN_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showMtu"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_MTU_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_MTU_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showSpeed"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_SPEED_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_SPEED_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showFlags"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_FLAGS_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_FLAGS_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "compact"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_COMPACT_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_COMPACT_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "defaultRouteOnly"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showAllIps"))
        {
            if (yyjson_get_bool(val))
                options->showType |= FF_LOCALIP_TYPE_ALL_IPS_BIT;
            else
                options->showType &= ~FF_LOCALIP_TYPE_ALL_IPS_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "namePrefix"))
        {
            ffStrbufSetJsonVal(&options->namePrefix, val);
            continue;
        }

        ffPrintError(FF_LOCALIP_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateLocalIpJsonConfig(FFLocalIpOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_bool(doc, module, "showIpv4", !!(options->showType & FF_LOCALIP_TYPE_IPV4_BIT));

    if (options->ipv6Type == FF_LOCALIP_IPV6_TYPE_AUTO)
        yyjson_mut_obj_add_bool(doc, module, "showIpv6", !!(options->showType & FF_LOCALIP_TYPE_IPV6_BIT));
    else
    {
        const char* str = NULL;
        switch (options->ipv6Type)
        {
            case FF_LOCALIP_IPV6_TYPE_GUA_BIT:     str = "gua"; break;
            case FF_LOCALIP_IPV6_TYPE_ULA_BIT:     str = "ula"; break;
            case FF_LOCALIP_IPV6_TYPE_LLA_BIT:     str = "lla"; break;
            case FF_LOCALIP_IPV6_TYPE_UNKNOWN_BIT: str = "unknown"; break;
            default:                               str = "auto"; break;
        }
        yyjson_mut_obj_add_str(doc, module, "showIpv6", str);
    }

    yyjson_mut_obj_add_bool(doc, module, "showMac", !!(options->showType & FF_LOCALIP_TYPE_MAC_BIT));

    yyjson_mut_obj_add_bool(doc, module, "showLoop", !!(options->showType & FF_LOCALIP_TYPE_LOOP_BIT));

    yyjson_mut_obj_add_bool(doc, module, "showPrefixLen", !!(options->showType & FF_LOCALIP_TYPE_PREFIX_LEN_BIT));

    yyjson_mut_obj_add_bool(doc, module, "showMtu", !!(options->showType & FF_LOCALIP_TYPE_MTU_BIT));

    yyjson_mut_obj_add_bool(doc, module, "showSpeed", !!(options->showType & FF_LOCALIP_TYPE_SPEED_BIT));

    yyjson_mut_obj_add_bool(doc, module, "showFlags", !!(options->showType & FF_LOCALIP_TYPE_FLAGS_BIT));

    yyjson_mut_obj_add_bool(doc, module, "compact", !!(options->showType & FF_LOCALIP_TYPE_COMPACT_BIT));

    yyjson_mut_obj_add_bool(doc, module, "defaultRouteOnly", !!(options->showType & FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT));

    yyjson_mut_obj_add_bool(doc, module, "showAllIps", !!(options->showType & FF_LOCALIP_TYPE_ALL_IPS_BIT));

    yyjson_mut_obj_add_strbuf(doc, module, "namePrefix", &options->namePrefix);
}

bool ffGenerateLocalIpJsonResult(FF_MAYBE_UNUSED FFLocalIpOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFLocalIpResult));

    const char* error = ffDetectLocalIps(options, &results);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &ip->name);
        if (options->showType & (FF_LOCALIP_TYPE_IPV4_BIT | FF_LOCALIP_TYPE_IPV6_BIT))
        {
            yyjson_mut_val* defaultRoute = yyjson_mut_obj_add_obj(doc, obj, "defaultRoute");
            if (options->showType & FF_LOCALIP_TYPE_IPV4_BIT)
                yyjson_mut_obj_add_bool(doc, defaultRoute, "ipv4", !!(ip->defaultRoute & FF_LOCALIP_TYPE_IPV4_BIT));
            if (options->showType & FF_LOCALIP_TYPE_IPV6_BIT)
                yyjson_mut_obj_add_bool(doc, defaultRoute, "ipv6", !!(ip->defaultRoute & FF_LOCALIP_TYPE_IPV6_BIT));
        }
        if (options->showType & FF_LOCALIP_TYPE_IPV4_BIT)
            yyjson_mut_obj_add_strbuf(doc, obj, "ipv4", &ip->ipv4);
        if (options->showType & FF_LOCALIP_TYPE_IPV6_BIT)
            yyjson_mut_obj_add_strbuf(doc, obj, "ipv6", &ip->ipv6);
        if (options->showType & FF_LOCALIP_TYPE_MAC_BIT)
            yyjson_mut_obj_add_strbuf(doc, obj, "mac", &ip->mac);
        if (options->showType & FF_LOCALIP_TYPE_MTU_BIT)
            yyjson_mut_obj_add_int(doc, obj, "mtu", ip->mtu);
        if (options->showType & FF_LOCALIP_TYPE_SPEED_BIT)
            yyjson_mut_obj_add_int(doc, obj, "speed", ip->speed);
        if (options->showType & FF_LOCALIP_TYPE_FLAGS_BIT)
            yyjson_mut_obj_add_strbuf(doc, obj, "flags", &ip->flags);
    }

    FF_LIST_FOR_EACH(FFLocalIpResult, ip, results)
    {
        ffStrbufDestroy(&ip->name);
        ffStrbufDestroy(&ip->ipv4);
        ffStrbufDestroy(&ip->ipv6);
        ffStrbufDestroy(&ip->mac);
        ffStrbufDestroy(&ip->flags);
    }

    return true;
}

void ffInitLocalIpOptions(FFLocalIpOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰩟");

    options->showType = FF_LOCALIP_TYPE_IPV4_BIT | FF_LOCALIP_TYPE_PREFIX_LEN_BIT
        #if !__ANDROID__ /*Permission denied*/
            | FF_LOCALIP_TYPE_DEFAULT_ROUTE_ONLY_BIT
        #endif
    ;
    options->ipv6Type = FF_LOCALIP_IPV6_TYPE_AUTO;
    ffStrbufInit(&options->namePrefix);
}

void ffDestroyLocalIpOptions(FFLocalIpOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->namePrefix);
}

FFModuleBaseInfo ffLocalIPModuleInfo = {
    .name = FF_LOCALIP_MODULE_NAME,
    .description = "List local IP addresses (v4 or v6), MAC addresses, etc",
    .initOptions = (void*) ffInitLocalIpOptions,
    .destroyOptions = (void*) ffDestroyLocalIpOptions,
    .parseJsonObject = (void*) ffParseLocalIpJsonObject,
    .printModule = (void*) ffPrintLocalIp,
    .generateJsonResult = (void*) ffGenerateLocalIpJsonResult,
    .generateJsonConfig = (void*) ffGenerateLocalIpJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"IPv4 address", "ipv4"},
        {"IPv6 address", "ipv6"},
        {"MAC address", "mac"},
        {"Interface name", "ifname"},
        {"Is default route", "is-default-route"},
        {"MTU size in bytes", "mtu"},
        {"Link speed (formatted)", "speed"},
        {"Interface flags", "flags"},
    }))
};
