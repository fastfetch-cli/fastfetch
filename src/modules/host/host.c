#include "fastfetch.h"
#include "common/printing.h"
#include "detection/host/host.h"
#include "modules/host/host.h"

#define FF_HOST_MODULE_NAME "Host"
#define FF_HOST_NUM_FORMAT_ARGS 5

void ffPrintHost(FFinstance* instance, FFHostOptions* options)
{
    const FFHostResult* host = ffDetectHost();

    if(host->error.length > 0)
    {
        ffPrintError(instance, FF_HOST_MODULE_NAME, 0, &options->moduleArgs, "%*s", host->error.length, host->error.chars);
        return;
    }

    if(host->productFamily.length == 0 && host->productName.length == 0)
    {
        ffPrintError(instance, FF_HOST_MODULE_NAME, 0, &options->moduleArgs, "neither product_family nor product_name is set by O.E.M.");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_HOST_MODULE_NAME, 0, &options->moduleArgs.key);

        FFstrbuf output;
        ffStrbufInit(&output);

        if(host->productName.length > 0)
            ffStrbufAppend(&output, &host->productName);
        else
            ffStrbufAppend(&output, &host->productFamily);

        if(host->productVersion.length > 0 && !ffStrbufIgnCaseEqualS(&host->productVersion, "none"))
        {
            ffStrbufAppendF(&output, " (%s)", host->productVersion.chars);
        }

        ffStrbufPutTo(&output, stdout);

        ffStrbufDestroy(&output);
    }
    else
    {
        ffPrintFormat(instance, FF_HOST_MODULE_NAME, 0, &options->moduleArgs, FF_HOST_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->productFamily},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->productName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->productVersion},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->productSku},
            {FF_FORMAT_ARG_TYPE_STRBUF, &host->sysVendor}
        });
    }
}

void ffInitHostOptions(FFHostOptions* options)
{
    options->moduleName = FF_HOST_MODULE_NAME;
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

#ifdef FF_HAVE_JSONC
bool ffParseHostJsonObject(FFinstance* instance, const char* type, JSONCData* data, json_object* module)
{
    if (strcasecmp(type, FF_HOST_MODULE_NAME) != 0)
        return false;

    FFHostOptions __attribute__((__cleanup__(ffDestroyHostOptions))) options;
    ffInitHostOptions(&options);

    FF_JSON_OBJECT_OBJECT_FOREACH(data, module, key, val)
    {
        if (strcasecmp(key, "type") == 0)
            continue;

        if (ffJsonConfigParseModuleArgs(data, key, val, &options.moduleArgs))
            continue;

        ffPrintError(instance, FF_HOST_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
    }

    ffPrintHost(instance, &options);
    return true;
}
#endif
