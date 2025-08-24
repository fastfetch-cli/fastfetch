#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/publicip/publicip.h"
#include "detection/publicip/publicip.h"
#include "util/stringUtils.h"

#define FF_PUBLICIP_DISPLAY_NAME "Public IP"

bool ffPrintPublicIp(FFPublicIPOptions* options)
{
    FFPublicIpResult result;
    ffStrbufInit(&result.ip);
    ffStrbufInit(&result.location);
    const char* error = ffDetectPublicIp(options, &result);

    if (error)
    {
        ffPrintError(FF_PUBLICIP_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_PUBLICIP_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        if (result.location.length)
            printf("%s (%s)\n", result.ip.chars, result.location.chars);
        else
            ffStrbufPutTo(&result.ip, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_PUBLICIP_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(result.ip, "ip"),
            FF_FORMAT_ARG(result.location, "location"),
        }));
    }

    ffStrbufDestroy(&result.ip);
    ffStrbufDestroy(&result.location);

    return true;
}

void ffParsePublicIpJsonObject(FFPublicIPOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "url"))
        {
            ffStrbufSetJsonVal(&options->url, val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "timeout"))
        {
            options->timeout = (uint32_t) yyjson_get_uint(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "ipv6"))
        {
            options->ipv6 = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_PUBLICIP_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGeneratePublicIpJsonConfig(FFPublicIPOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_strbuf(doc, module, "url", &options->url);

    yyjson_mut_obj_add_uint(doc, module, "timeout", options->timeout);

    yyjson_mut_obj_add_bool(doc, module, "ipv6", options->ipv6);
}

bool ffGeneratePublicIpJsonResult(FFPublicIPOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFPublicIpResult result;
    ffStrbufInit(&result.ip);
    ffStrbufInit(&result.location);
    const char* error = ffDetectPublicIp(options, &result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "ip", &result.ip);
    yyjson_mut_obj_add_strbuf(doc, obj, "location", &result.location);

    ffStrbufDestroy(&result.ip);
    ffStrbufDestroy(&result.location);

    return true;
}

void ffInitPublicIpOptions(FFPublicIPOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰩠");

    ffStrbufInit(&options->url);
    options->timeout = 0;
    options->ipv6 = false;
}

void ffDestroyPublicIpOptions(FFPublicIPOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);

    ffStrbufDestroy(&options->url);
}

FFModuleBaseInfo ffPublicIPModuleInfo = {
    .name = FF_PUBLICIP_MODULE_NAME,
    .description = "Print your public IP address, etc",
    .initOptions = (void*) ffInitPublicIpOptions,
    .destroyOptions = (void*) ffDestroyPublicIpOptions,
    .parseJsonObject = (void*) ffParsePublicIpJsonObject,
    .printModule = (void*) ffPrintPublicIp,
    .generateJsonResult = (void*) ffGeneratePublicIpJsonResult,
    .generateJsonConfig = (void*) ffGeneratePublicIpJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Public IP address", "ip"},
        {"Location", "location"},
    }))
};
