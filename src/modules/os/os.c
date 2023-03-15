#include "fastfetch.h"
#include "common/printing.h"
#include "common/option.h"
#include "detection/os/os.h"
#include "modules/os/os.h"

#include <ctype.h>

#define FF_OS_NUM_FORMAT_ARGS 12

static void buildOutputDefault(const FFinstance* instance, const FFOSResult* os, FFstrbuf* result)
{
    //Create the basic output
    if(os->name.length > 0)
        ffStrbufAppend(result, &os->name);
    else if(os->prettyName.length > 0)
        ffStrbufAppend(result, &os->prettyName);
    else if(os->id.length > 0)
        ffStrbufAppend(result, &os->id);
    else
        ffStrbufAppend(result, &instance->state.platform.systemName);

    #ifdef __APPLE__
    if(os->codename.length > 0)
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &os->codename);
    }
    #endif

    //Append version if it is missing
    if(os->versionID.length > 0 && ffStrbufFirstIndex(result, &os->versionID) == result->length)
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &os->versionID);
    }
    else if(os->versionID.length == 0 && os->version.length > 0 && ffStrbufFirstIndex(result, &os->version) == result->length)
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &os->version);
    }

    #ifdef __APPLE__
    if(os->buildID.length > 0)
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &os->buildID);
    }
    #endif

    //Append variant if it is missing
    if(os->variant.length > 0 && ffStrbufFirstIndex(result, &os->variant) == result->length)
    {
        ffStrbufAppendS(result, " (");
        ffStrbufAppend(result, &os->variant);
        ffStrbufAppendC(result, ')');
    }
    else if(os->variant.length == 0 && os->variantID.length > 0 && ffStrbufFirstIndex(result, &os->variantID) == result->length)
    {
        ffStrbufAppendS(result, " (");
        ffStrbufAppend(result, &os->variantID);
        ffStrbufAppendC(result, ')');
    }

    //Append architecture if it is missing
    if(ffStrbufFirstIndex(result, &instance->state.platform.systemArchitecture) == result->length)
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &instance->state.platform.systemArchitecture);
    }
}

static void buildOutputNixOS(const FFinstance* instance, const FFOSResult* os, FFstrbuf* result)
{
    ffStrbufAppendS(result, "NixOS");

    if(os->buildID.length > 0)
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &os->buildID);
    }

    if(os->codename.length > 0)
    {
        ffStrbufAppendS(result, " (");
        ffStrbufAppendC(result, (char) toupper(os->codename.chars[0]));
        ffStrbufAppendS(result, os->codename.chars + 1);
        ffStrbufAppendC(result, ')');
    }

    if(instance->state.platform.systemArchitecture.length > 0)
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &instance->state.platform.systemArchitecture);
    }
}

void ffPrintOS(FFinstance* instance, FFOSOptions* options)
{
    const FFOSResult* os = ffDetectOS(instance);

    if(os->name.length == 0 && os->prettyName.length == 0 && os->id.length == 0)
    {
        ffPrintError(instance, FF_OS_MODULE_NAME, 0, &options->moduleArgs, "Could not detect OS");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        FF_STRBUF_AUTO_DESTROY result;
        ffStrbufInit(&result);

        if(ffStrbufIgnCaseCompS(&os->id, "nixos") == 0)
            buildOutputNixOS(instance, os, &result);
        else
            buildOutputDefault(instance, os, &result);

        ffPrintLogoAndKey(instance, FF_OS_MODULE_NAME, 0, &options->moduleArgs.key);
        ffStrbufPutTo(&result, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_OS_MODULE_NAME, 0, &options->moduleArgs, FF_OS_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &os->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &os->prettyName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &os->id},
            {FF_FORMAT_ARG_TYPE_STRBUF, &os->idLike},
            {FF_FORMAT_ARG_TYPE_STRBUF, &os->variant},
            {FF_FORMAT_ARG_TYPE_STRBUF, &os->variantID},
            {FF_FORMAT_ARG_TYPE_STRBUF, &os->version},
            {FF_FORMAT_ARG_TYPE_STRBUF, &os->versionID},
            {FF_FORMAT_ARG_TYPE_STRBUF, &os->codename},
            {FF_FORMAT_ARG_TYPE_STRBUF, &os->buildID},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.platform.systemArchitecture}
        });
    }
}

void ffInitOSOptions(FFOSOptions* options)
{
    options->moduleName = FF_OS_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);

    #if defined(__linux__) || defined(__FreeBSD__)
    ffStrbufInit(&options->file);
    #endif
}

bool ffParseOSCommandOptions(FFOSOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_OS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    #if defined(__linux__) || defined(__FreeBSD__)
        if (strcasecmp(subKey, "file") == 0)
        {
            ffOptionParseString(key, value, &options->file);
            return true;
        }
    #endif

    return false;
}

void ffDestroyOSOptions(FFOSOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    #if defined(__linux__) || defined(__FreeBSD__)
        ffStrbufDestroy(&options->file);
    #endif
}

#ifdef FF_HAVE_JSONC
void ffParseOSJsonObject(FFinstance* instance, json_object* module)
{
    FFOSOptions __attribute__((__cleanup__(ffDestroyOSOptions))) options;
    ffInitOSOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            #if defined(__linux__) || defined(__FreeBSD__)
                if (strcasecmp(key, "file") == 0)
                {
                    ffStrbufSetS(&options.file, json_object_get_string(val));
                    continue;
                }
            #endif

            ffPrintError(instance, FF_OS_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintOS(instance, &options);
}
#endif
