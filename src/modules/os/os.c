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

        ffPrintLogoAndKey(FF_OS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&result, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_OS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
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
            FF_FORMAT_ARG(instance.state.platform.sysinfo.architecture, "arch")
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

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_OS_MODULE_NAME,
    .description = "Print operating system name and version",
    .parseCommandOptions = (void*) ffParseOSCommandOptions,
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

void ffInitOSOptions(FFOSOptions* options)
{
    options->moduleInfo = ffModuleInfo;
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
        #else
            "?"
        #endif
    );
}

void ffDestroyOSOptions(FFOSOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
