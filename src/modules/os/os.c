#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/option.h"
#include "detection/os/os.h"
#include "modules/os/os.h"
#include "util/stringUtils.h"

#include <ctype.h>

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
        ffStrbufAppend(result, &instance.state.platform.sysinfo.name);

    //Append code name if it is missing
    if(os->codename.length > 0 && !ffStrbufContainIgnCase(result, &os->codename))
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &os->codename);
    }

    //Append version if it is missing
    if(os->versionID.length > 0 && !ffStrbufContainIgnCase(result, &os->versionID))
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &os->versionID);
    }
    else if(os->versionID.length == 0 && os->version.length > 0 && !ffStrbufContainIgnCase(result, &os->version))
    {
        ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &os->version);
    }

    //Append variant if it is missing
    if(os->variant.length > 0 && !ffStrbufContainIgnCase(result, &os->variant))
    {
        ffStrbufAppendS(result, " (");
        ffStrbufAppend(result, &os->variant);
        ffStrbufAppendC(result, ')');
    }
    else if(os->variant.length == 0 && os->variantID.length > 0 && !ffStrbufContainIgnCase(result, &os->variantID))
    {
        ffStrbufAppendS(result, " (");
        ffStrbufAppend(result, &os->variantID);
        ffStrbufAppendC(result, ')');
    }
}

bool ffPrintOS(FFOSOptions* options)
{
    const FFOSResult* os = ffDetectOS();

    if(os->name.length == 0 && os->prettyName.length == 0 && os->id.length == 0)
    {
        ffPrintError(FF_OS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Could not detect OS");
        return false;
    }

    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    if(options->moduleArgs.key.length == 0)
        ffStrbufSetStatic(&key, FF_OS_MODULE_NAME);
    else
    {
        FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, ((FFformatarg[]) {
            FF_FORMAT_ARG(instance.state.platform.sysinfo.name, "sysname"),
            FF_FORMAT_ARG(os->name, "name"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();

        if(os->prettyName.length > 0)
            ffStrbufAppend(&result, &os->prettyName);
        else
            buildOutputDefault(os, &result);

        //Append architecture if it is missing
        if(!ffStrbufContainIgnCase(&result, &instance.state.platform.sysinfo.architecture))
        {
            ffStrbufAppendC(&result, ' ');
            ffStrbufAppend(&result, &instance.state.platform.sysinfo.architecture);
        }

        ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);
        ffStrbufPutTo(&result, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]){
            FF_FORMAT_ARG(instance.state.platform.sysinfo.name, "sysname"),
            FF_FORMAT_ARG(os->name, "name"),
            FF_FORMAT_ARG(os->prettyName, "pretty-name"),
            FF_FORMAT_ARG(os->id, "id"),
            FF_FORMAT_ARG(os->idLike, "id-like"),
            FF_FORMAT_ARG(os->variant, "variant"),
            FF_FORMAT_ARG(os->variantID, "variant-id"),
            FF_FORMAT_ARG(os->version, "version"),
            FF_FORMAT_ARG(os->versionID, "version-id"),
            FF_FORMAT_ARG(os->codename, "codename"),
            FF_FORMAT_ARG(os->buildID, "build-id"),
            FF_FORMAT_ARG(instance.state.platform.sysinfo.architecture, "arch"),
            FF_FORMAT_ARG(instance.state.platform.sysinfo.release, "kernel-release"),
        }));
    }

    return true;
}

void ffParseOSJsonObject(FFOSOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_OS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateOSJsonConfig(FFOSOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateOSJsonResult(FF_MAYBE_UNUSED FFOSOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFOSResult* os = ffDetectOS();

    if(os->name.length == 0 && os->prettyName.length == 0 && os->id.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "Could not detect OS");
        return false;
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

    return true;
}

void ffInitOSOptions(FFOSOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs,
        #ifdef _WIN32
            ""
        #elif __APPLE__
            ""
        #elif __FreeBSD__
            "󰣠"
        #elif __ANDROID__
            ""
        #elif __linux__
            ""
        #elif __sun
            ""
        #elif __OpenBSD__
            ""
        #elif __Haiku__
            ""
        #else
            "󰢻"
        #endif
    );
}

void ffDestroyOSOptions(FFOSOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffOSModuleInfo = {
    .name = FF_OS_MODULE_NAME,
    .description = "Print operating system name and version",
    .initOptions = (void*) ffInitOSOptions,
    .destroyOptions = (void*) ffDestroyOSOptions,
    .parseJsonObject = (void*) ffParseOSJsonObject,
    .printModule = (void*) ffPrintOS,
    .generateJsonResult = (void*) ffGenerateOSJsonResult,
    .generateJsonConfig = (void*) ffGenerateOSJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Name of the kernel", "sysname"},
        {"Name of the OS", "name"},
        {"Pretty name of the OS, if available", "pretty-name"},
        {"ID of the OS", "id"},
        {"ID like of the OS", "id-like"},
        {"Variant of the OS", "variant"},
        {"Variant ID of the OS", "variant-id"},
        {"Version of the OS", "version"},
        {"Version ID of the OS", "version-id"},
        {"Version codename of the OS", "codename"},
        {"Build ID of the OS", "build-id"},
        {"Architecture of the OS", "arch"},
    }))
};
