#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/dns/dns.h"
#include "modules/dns/dns.h"
#include "util/stringUtils.h"

bool ffPrintDNS(FFDNSOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFstrbuf));

    const char* error = ffDetectDNS(options, &result);

    if (error)
    {
        ffPrintError(FF_DNS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (result.length == 0)
    {
        ffPrintError(FF_DNS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "NO DNS servers detected");
        return false;
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
        FF_PRINT_FORMAT_CHECKED(FF_DNS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(buf, "result"),
        }));
    }

    FF_LIST_FOR_EACH(FFstrbuf, item, result)
    {
        ffStrbufDestroy(item);
    }

    return true;
}

void ffParseDNSJsonObject(FFDNSOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "showType"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "both", FF_DNS_TYPE_BOTH },
                { "ipv4", FF_DNS_TYPE_IPV4_BIT },
                { "ipv6", FF_DNS_TYPE_IPV6_BIT },
                {},
            });
            if (error)
                ffPrintError(FF_DNS_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
            else
                options->showType = (FFDNSShowType) value;
            continue;
        }

        ffPrintError(FF_DNS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateDNSJsonConfig(FFDNSOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    switch ((uint8_t) options->showType)
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

bool ffGenerateDNSJsonResult(FFDNSOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFstrbuf));

    const char* error = ffDetectDNS(options, &result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH(FFstrbuf, item, result)
    {
        yyjson_mut_arr_add_strbuf(doc, arr, item);
    }

    FF_LIST_FOR_EACH(FFstrbuf, item, result)
    {
        ffStrbufDestroy(item);
    }

    return true;
}

void ffInitDNSOptions(FFDNSOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰇖");

    options->showType = FF_DNS_TYPE_BOTH;
}

void ffDestroyDNSOptions(FFDNSOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffDNSModuleInfo = {
    .name = FF_DNS_MODULE_NAME,
    .description = "Print configured DNS servers",
    .initOptions = (void*) ffInitDNSOptions,
    .destroyOptions = (void*) ffDestroyDNSOptions,
    .parseJsonObject = (void*) ffParseDNSJsonObject,
    .printModule = (void*) ffPrintDNS,
    .generateJsonResult = (void*) ffGenerateDNSJsonResult,
    .generateJsonConfig = (void*) ffGenerateDNSJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"DNS result", "result"},
    }))
};
