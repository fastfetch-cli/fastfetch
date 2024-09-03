#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/dns/dns.h"
#include "modules/dns/dns.h"
#include "util/stringUtils.h"

#define FF_DNS_NUM_FORMAT_ARGS 1

void ffPrintDNS(FFDNSOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFstrbuf));

    const char* error = ffDetectDNS(options, &result);

    if (error)
    {
        ffPrintError(FF_DNS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if (result.length == 0)
    {
        ffPrintError(FF_DNS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "NO DNS servers detected");
        return;
    }

    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
    FF_LIST_FOR_EACH(FFstrbuf, item, result)
    {
        if (!ffStrbufContainC(item, '.')) continue; // IPv4
        if (buf.length)
            ffStrbufAppendC(&buf, ' ');
        ffStrbufAppend(&buf, item);
    }
    FF_LIST_FOR_EACH(FFstrbuf, item, result)
    {
        if (!ffStrbufContainC(item, ':')) continue; // IPv6
        if (buf.length)
            ffStrbufAppendC(&buf, ' ');
        ffStrbufAppend(&buf, item);
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_DNS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        ffStrbufPutTo(&buf, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_DNS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_DNS_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            FF_FORMAT_ARG(buf, "result"),
        }));
    }

    FF_LIST_FOR_EACH(FFstrbuf, item, result)
    {
        ffStrbufDestroy(item);
    }
}

bool ffParseDNSCommandOptions(FFDNSOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_DNS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "show-type"))
    {
        options->showType = (FFDNSShowType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "both", FF_DNS_TYPE_BOTH },
            { "ipv4", FF_DNS_TYPE_IPV4_BIT },
            { "ipv6", FF_DNS_TYPE_IPV6_BIT },
            {},
        });
        return true;
    }

    return false;
}

void ffParseDNSJsonObject(FFDNSOptions* options, yyjson_val* module)
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

        if (ffStrEqualsIgnCase(key, "showType"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "both", FF_DNS_TYPE_BOTH },
                { "ipv4", FF_DNS_TYPE_IPV4_BIT },
                { "ipv6", FF_DNS_TYPE_IPV6_BIT },
                {},
            });
            if (error)
                ffPrintError(FF_DNS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s value: %s", key, error);
            else
                options->showType = (FFDNSShowType) value;
            continue;
        }

        ffPrintError(FF_DNS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateDNSJsonConfig(FFDNSOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyDNSOptions))) FFDNSOptions defaultOptions;
    ffInitDNSOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (defaultOptions.showType != options->showType)
    {
        switch (options->showType)
        {
            case FF_DNS_TYPE_IPV4_BIT:
                yyjson_mut_obj_add_str(doc, module, "showType", "ipv4");
                break;
            case FF_DNS_TYPE_IPV6_BIT:
                yyjson_mut_obj_add_str(doc, module, "showType", "ipv6");
                break;
            case FF_DNS_TYPE_BOTH:
                yyjson_mut_obj_add_str(doc, module, "showType", "both");
                break;
        }
    }
}

void ffGenerateDNSJsonResult(FF_MAYBE_UNUSED FFDNSOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFstrbuf));

    const char* error = ffDetectDNS(options, &result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH(FFstrbuf, item, result)
    {
        yyjson_mut_arr_add_strbuf(doc, arr, item);
    }

exit:
    FF_LIST_FOR_EACH(FFstrbuf, item, result)
    {
        ffStrbufDestroy(item);
    }
}

void ffPrintDNSHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_DNS_MODULE_NAME, "{1}", FF_DNS_NUM_FORMAT_ARGS, ((const char* []) {
        "DNS result - result",
    }));
}

void ffInitDNSOptions(FFDNSOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_DNS_MODULE_NAME,
        "Print configured DNS servers",
        ffParseDNSCommandOptions,
        ffParseDNSJsonObject,
        ffPrintDNS,
        ffGenerateDNSJsonResult,
        ffPrintDNSHelpFormat,
        ffGenerateDNSJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "󰇖");

    options->showType = FF_DNS_TYPE_BOTH;
}

void ffDestroyDNSOptions(FFDNSOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
