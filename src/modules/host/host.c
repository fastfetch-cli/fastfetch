#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/host/host.h"
#include "modules/host/host.h"
#include "util/stringUtils.h"

#define FF_HOST_NUM_FORMAT_ARGS 5

void ffPrintHost(FFHostOptions* options)
{
    FFHostResult host;
    ffStrbufInit(&host.productFamily);
    ffStrbufInit(&host.productName);
    ffStrbufInit(&host.productVersion);
    ffStrbufInit(&host.productSku);
    ffStrbufInit(&host.sysVendor);
    const char* error = ffDetectHost(&host);

    if(error)
    {
        ffPrintError(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        goto exit;
    }

    if(host.productFamily.length == 0 && host.productName.length == 0)
    {
        ffPrintError(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, "neither product_family nor product_name is set by O.E.M.");
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY output = ffStrbufCreate();

        if(host.productName.length > 0)
            ffStrbufAppend(&output, &host.productName);
        else
            ffStrbufAppend(&output, &host.productFamily);

        if(host.productVersion.length > 0 && !ffStrbufIgnCaseEqualS(&host.productVersion, "none"))
        {
            ffStrbufAppendF(&output, " (%s)", host.productVersion.chars);
        }

        ffStrbufPutTo(&output, stdout);
    }
    else
    {
        ffPrintFormat(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, FF_HOST_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.productFamily},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.productName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.productVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.productSku},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host.sysVendor}
        });
    }

exit:
    ffStrbufDestroy(&host.productFamily);
    ffStrbufDestroy(&host.productName);
    ffStrbufDestroy(&host.productVersion);
    ffStrbufDestroy(&host.productSku);
    ffStrbufDestroy(&host.sysVendor);
}

void ffInitHostOptions(FFHostOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_HOST_MODULE_NAME, ffParseHostCommandOptions, ffParseHostJsonObject, ffPrintHost, ffGenerateHostJsonResult, ffPrintHostHelpFormat);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseHostCommandOptions(FFHostOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_HOST_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyHostOptions(FFHostOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
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

        ffPrintError(FF_HOST_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateHostJsonResult(FF_MAYBE_UNUSED FFHostOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFHostResult host;
    ffStrbufInit(&host.productFamily);
    ffStrbufInit(&host.productName);
    ffStrbufInit(&host.productVersion);
    ffStrbufInit(&host.productSku);
    ffStrbufInit(&host.sysVendor);
    const char* error = ffDetectHost(&host);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    if (host.productFamily.length == 0 && host.productName.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "neither product_family nor product_name is set by O.E.M.");
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "family", &host.productFamily);
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &host.productName);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &host.productVersion);
    yyjson_mut_obj_add_strbuf(doc, obj, "sku", &host.productSku);
    yyjson_mut_obj_add_strbuf(doc, obj, "sysVender", &host.sysVendor);

exit:
    ffStrbufDestroy(&host.productFamily);
    ffStrbufDestroy(&host.productName);
    ffStrbufDestroy(&host.productVersion);
    ffStrbufDestroy(&host.productSku);
    ffStrbufDestroy(&host.sysVendor);
}

void ffPrintHostHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_HOST_MODULE_NAME, "{2} {3}", FF_HOST_NUM_FORMAT_ARGS, (const char* []) {
        "product family",
        "product name",
        "product version",
        "product sku",
        "sys vendor"
    });
}
