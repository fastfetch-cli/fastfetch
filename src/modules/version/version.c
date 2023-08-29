#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/version/version.h"
#include "modules/version/version.h"
#include "util/stringUtils.h"

#define FF_VERSION_NUM_FORMAT_ARGS 6

void ffPrintVersion(FFVersionOptions* options)
{
    FFVersionResult result = {};
    ffDetectVersion(&result);

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_VERSION_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        printf("%s %s%s%s (%s)\n", result.projectName, result.version, result.versionTweak, result.debugMode ? "-debug" : "", result.architecture);
    }
    else
    {
        ffPrintFormat(FF_VERSION_MODULE_NAME, 0, &options->moduleArgs, FF_VERSION_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, result.projectName},
            {FF_FORMAT_ARG_TYPE_STRING, result.version},
            {FF_FORMAT_ARG_TYPE_STRING, result.versionTweak},
            {FF_FORMAT_ARG_TYPE_STRING, result.debugMode ? "debug" : "release"},
            {FF_FORMAT_ARG_TYPE_STRING, result.architecture},
            {FF_FORMAT_ARG_TYPE_STRING, result.cmakeBuiltType},
        });
    }
}

void ffInitVersionOptions(FFVersionOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_VERSION_MODULE_NAME, ffParseVersionCommandOptions, ffParseVersionJsonObject, ffPrintVersion, NULL);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseVersionCommandOptions(FFVersionOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_VERSION_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyVersionOptions(FFVersionOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseVersionJsonObject(FFVersionOptions* options, yyjson_val* module)
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

        ffPrintError(FF_VERSION_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}
