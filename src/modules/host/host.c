#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/host/host.h"
#include "modules/host/host.h"
#include "util/stringUtils.h"

#define FF_HOST_NUM_FORMAT_ARGS 7

void ffPrintHost(FFHostOptions* options)
{
    FFHostResult host;
    ffStrbufInit(&host.family);
    ffStrbufInit(&host.name);
    ffStrbufInit(&host.version);
    ffStrbufInit(&host.sku);
    ffStrbufInit(&host.serial);
    ffStrbufInit(&host.uuid);
    ffStrbufInit(&host.vendor);

    const char* error = ffDetectHost(&host);
    if(error)
    {
        ffPrintError(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if(host.family.length == 0 && host.name.length == 0)
    {
        ffPrintError(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "neither product_family nor product_name is set by O.E.M.");
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY output = ffStrbufCreate();

        if(host.name.length > 0)
            ffStrbufAppend(&output, &host.name);
        else
            ffStrbufAppend(&output, &host.family);

        if(host.version.length > 0)
            ffStrbufAppendF(&output, " (%s)", host.version.chars);

        ffStrbufPutTo(&output, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_HOST_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.family, "family"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.name, "name"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.version, "version"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.sku, "sku"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.vendor, "vendor"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.serial, "serial"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.uuid, "uuid"},
        }));
    }

exit:
    ffStrbufDestroy(&host.family);
    ffStrbufDestroy(&host.name);
    ffStrbufDestroy(&host.version);
    ffStrbufDestroy(&host.sku);
    ffStrbufDestroy(&host.serial);
    ffStrbufDestroy(&host.uuid);
    ffStrbufDestroy(&host.vendor);
}

bool ffParseHostCommandOptions(FFHostOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_HOST_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseHostJsonObject(FFHostOptions* options, yyjson_val* module)
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

        ffPrintError(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateHostJsonConfig(FFHostOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyHostOptions))) FFHostOptions defaultOptions;
    ffInitHostOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateHostJsonResult(FF_MAYBE_UNUSED FFHostOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFHostResult host;
    ffStrbufInit(&host.family);
    ffStrbufInit(&host.name);
    ffStrbufInit(&host.version);
    ffStrbufInit(&host.sku);
    ffStrbufInit(&host.serial);
    ffStrbufInit(&host.uuid);
    ffStrbufInit(&host.vendor);

    const char* error = ffDetectHost(&host);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    if (host.family.length == 0 && host.name.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "neither product_family nor product_name is set by O.E.M.");
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "family", &host.family);
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &host.name);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &host.version);
    yyjson_mut_obj_add_strbuf(doc, obj, "sku", &host.sku);
    yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &host.vendor);
    yyjson_mut_obj_add_strbuf(doc, obj, "serial", &host.serial);
    yyjson_mut_obj_add_strbuf(doc, obj, "uuid", &host.uuid);

exit:
    ffStrbufDestroy(&host.family);
    ffStrbufDestroy(&host.name);
    ffStrbufDestroy(&host.version);
    ffStrbufDestroy(&host.sku);
    ffStrbufDestroy(&host.serial);
    ffStrbufDestroy(&host.uuid);
    ffStrbufDestroy(&host.vendor);
}

void ffPrintHostHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_HOST_MODULE_NAME, "{2} {3}", FF_HOST_NUM_FORMAT_ARGS, ((const char* []) {
        "product family - family",
        "product name - name",
        "product version - version",
        "product sku - sku",
        "product vendor - vendor",
        "product serial number - serial",
        "product uuid - uuid",
    }));
}

void ffInitHostOptions(FFHostOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_HOST_MODULE_NAME,
        "Print product name of your computer",
        ffParseHostCommandOptions,
        ffParseHostJsonObject,
        ffPrintHost,
        ffGenerateHostJsonResult,
        ffPrintHostHelpFormat,
        ffGenerateHostJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyHostOptions(FFHostOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
