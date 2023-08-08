#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/nativeresolution/nativeresolution.h"
#include "modules/nativeresolution/nativeresolution.h"
#include "util/stringUtils.h"

#define FF_NATIVERESOLUTION_DISPLAY_NAME "Native Resolution"
#define FF_NATIVERESOLUTION_NUM_FORMAT_ARGS 3

void ffPrintNativeResolution(FFNativeResolutionOptions* options)
{
    FF_LIST_AUTO_DESTROY result = ffListCreate(sizeof(FFNativeResolutionResult));

    const char* error = ffDetectNativeResolution(&result);

    if(error)
    {
        ffPrintError(FF_NATIVERESOLUTION_DISPLAY_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(FF_NATIVERESOLUTION_DISPLAY_NAME, 0, &options->moduleArgs, "No devices detected");
        return;
    }

    uint8_t index = 0;
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    FF_LIST_FOR_EACH(FFNativeResolutionResult, display, result)
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffStrbufClear(&key);
            if(options->moduleArgs.key.length == 0)
            {
                ffStrbufAppendF(&key, "%s (%s)", FF_NATIVERESOLUTION_DISPLAY_NAME, display->name.chars);
            }
            else
            {
                ffParseFormatString(&key, &options->moduleArgs.key, 1, (FFformatarg[]){
                    {FF_FORMAT_ARG_TYPE_STRBUF, &display->name},
                });
            }
            ffPrintLogoAndKey(key.chars, 0, NULL, &options->moduleArgs.keyColor);

            printf("%ux%u", display->width, display->height);
        }
        else
        {
            ffPrintFormat(FF_NATIVERESOLUTION_DISPLAY_NAME, index, &options->moduleArgs, FF_NATIVERESOLUTION_NUM_FORMAT_ARGS, (FFformatarg[]) {
                {FF_FORMAT_ARG_TYPE_STRBUF, &display->name},
                {FF_FORMAT_ARG_TYPE_UINT, &display->width},
                {FF_FORMAT_ARG_TYPE_UINT, &display->height},
            });
        }

        ffStrbufDestroy(&display->name);
    }
}

void ffInitNativeResolutionOptions(FFNativeResolutionOptions* options)
{
    options->moduleName = FF_NATIVERESOLUTION_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseNativeResolutionCommandOptions(FFNativeResolutionOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_NATIVERESOLUTION_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyNativeResolutionOptions(FFNativeResolutionOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseNativeResolutionJsonObject(yyjson_val* module)
{
    FFNativeResolutionOptions __attribute__((__cleanup__(ffDestroyNativeResolutionOptions))) options;
    ffInitNativeResolutionOptions(&options);

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

            ffPrintError(FF_NATIVERESOLUTION_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintNativeResolution(&options);
}
