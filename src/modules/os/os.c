#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/option.h"
#include "detection/os/os.h"
#include "modules/os/os.h"
#include "util/stringUtils.h"

#include <ctype.h>

#define FF_OS_NUM_FORMAT_ARGS 12

static void buildOutputDefault(const FFOSResult* os, FFstrbuf* result)
{
    //Create the basic output
    if(os->name.length > 0)
        ffStrbufAppend(result, &os->name);
    else if(os->prettyName.length > 0)
        ffStrbufAppend(result, &os->prettyName);
    else if(os->id.length > 0)
        ffStrbufAppend(result, &os->id);
    else
        ffStrbufAppend(result, &instance.state.platform.systemName);

    //Append code name if it is missing
    if(os->codename.length > 0 && !ffStrbufContain(result, &os->codename))
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &os->codename);
    }

    //Append version if it is missing
    if(os->versionID.length > 0 && !ffStrbufContain(result, &os->versionID))
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &os->versionID);
    }
    else if(os->versionID.length == 0 && os->version.length > 0 && !ffStrbufContain(result, &os->version))
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
    if(ffStrbufFirstIndex(result, &instance.state.platform.systemArchitecture) == result->length)
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &instance.state.platform.systemArchitecture);
    }
}

static void buildOutputNixOS(const FFOSResult* os, FFstrbuf* result)
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

    if(instance.state.platform.systemArchitecture.length > 0)
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &instance.state.platform.systemArchitecture);
    }
}

void ffPrintOS(FFOSOptions* options)
{
    const FFOSResult* os = ffDetectOS();

    if(os->name.length == 0 && os->prettyName.length == 0 && os->id.length == 0)
    {
        ffPrintError(FF_OS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Could not detect OS");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();

        if(ffStrbufIgnCaseCompS(&os->id, "nixos") == 0)
            buildOutputNixOS(os, &result);
        else
            buildOutputDefault(os, &result);

        ffPrintLogoAndKey(FF_OS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&result, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_OS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_OS_NUM_FORMAT_ARGS, ((FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance.state.platform.systemName},
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
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance.state.platform.systemArchitecture}
        }));
    }
}

bool ffParseOSCommandOptions(FFOSOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_OS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseOSJsonObject(FFOSOptions* options, yyjson_val* module)
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

        ffPrintError(FF_OS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateOSJsonConfig(FFOSOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyOSOptions))) FFOSOptions defaultOptions;
    ffInitOSOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateOSJsonResult(FF_MAYBE_UNUSED FFOSOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFOSResult* os = ffDetectOS();

    if(os->name.length == 0 && os->prettyName.length == 0 && os->id.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Could not detect OS");
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "buildID", &os->buildID);
    yyjson_mut_obj_add_strbuf(doc, obj, "codename", &os->codename);
    yyjson_mut_obj_add_strbuf(doc, obj, "id", &os->id);
    yyjson_mut_obj_add_strbuf(doc, obj, "idLike", &os->idLike);
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &os->name);
    yyjson_mut_obj_add_strbuf(doc, obj, "prettyName", &os->prettyName);
    yyjson_mut_obj_add_strbuf(doc, obj, "variant", &os->variant);
    yyjson_mut_obj_add_strbuf(doc, obj, "variantID", &os->variantID);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &os->version);
    yyjson_mut_obj_add_strbuf(doc, obj, "versionID", &os->versionID);
}

void ffPrintOSHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_OS_MODULE_NAME, "{3} {10} {12}", FF_OS_NUM_FORMAT_ARGS, ((const char* []) {
        "Name of the kernel (Linux, WIN32_NT, Darwin, FreeBSD)",
        "Name of the OS",
        "Pretty name of the OS",
        "ID of the OS",
        "ID like of the OS",
        "Variant of the OS",
        "Variant ID of the OS",
        "Version of the OS",
        "Version ID of the OS",
        "Version codename of the OS",
        "Build ID of the OS",
        "Architecture of the OS"
    }));
}

void ffInitOSOptions(FFOSOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_OS_MODULE_NAME,
        "Print operating system name and version",
        ffParseOSCommandOptions,
        ffParseOSJsonObject,
        ffPrintOS,
        ffGenerateOSJsonResult,
        ffPrintOSHelpFormat,
        ffGenerateOSJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyOSOptions(FFOSOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
