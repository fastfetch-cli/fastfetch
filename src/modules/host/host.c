#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/host/host.h"
#include "modules/host/host.h"
#include "util/stringUtils.h"

bool ffPrintHost(FFHostOptions* options)
{
    bool success = false;
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
        FF_PRINT_FORMAT_CHECKED(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(host.family, "family"),
            FF_FORMAT_ARG(host.name, "name"),
            FF_FORMAT_ARG(host.version, "version"),
            FF_FORMAT_ARG(host.sku, "sku"),
            FF_FORMAT_ARG(host.vendor, "vendor"),
            FF_FORMAT_ARG(host.serial, "serial"),
            FF_FORMAT_ARG(host.uuid, "uuid"),
        }));
    }
    success = true;

exit:
    ffStrbufDestroy(&host.family);
    ffStrbufDestroy(&host.name);
    ffStrbufDestroy(&host.version);
    ffStrbufDestroy(&host.sku);
    ffStrbufDestroy(&host.serial);
    ffStrbufDestroy(&host.uuid);
    ffStrbufDestroy(&host.vendor);

    return success;
}

void ffParseHostJsonObject(FFHostOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateHostJsonConfig(FFHostOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateHostJsonResult(FF_MAYBE_UNUSED FFHostOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    bool success = false;
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
    success = true;

exit:
    ffStrbufDestroy(&host.family);
    ffStrbufDestroy(&host.name);
    ffStrbufDestroy(&host.version);
    ffStrbufDestroy(&host.sku);
    ffStrbufDestroy(&host.serial);
    ffStrbufDestroy(&host.uuid);
    ffStrbufDestroy(&host.vendor);

    return success;
}

void ffInitHostOptions(FFHostOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰌢");
}

void ffDestroyHostOptions(FFHostOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffHostModuleInfo = {
    .name = FF_HOST_MODULE_NAME,
    .description = "Print product name of your computer",
    .initOptions = (void*) ffInitHostOptions,
    .destroyOptions = (void*) ffDestroyHostOptions,
    .parseJsonObject = (void*) ffParseHostJsonObject,
    .printModule = (void*) ffPrintHost,
    .generateJsonResult = (void*) ffGenerateHostJsonResult,
    .generateJsonConfig = (void*) ffGenerateHostJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Product family", "family"},
        {"Product name", "name"},
        {"Product version", "version"},
        {"Product sku", "sku"},
        {"Product vendor", "vendor"},
        {"Product serial number", "serial"},
        {"Product uuid", "uuid"},
    }))
};
