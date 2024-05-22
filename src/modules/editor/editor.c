#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/libc/libc.h"
#include "detection/editor/editor.h"
#include "modules/editor/editor.h"
#include "util/stringUtils.h"

#define FF_EDITOR_NUM_FORMAT_ARGS 4

void ffPrintEditor(FFEditorOptions* options)
{
    FFEditorResult result = {
        .name = ffStrbufCreate(),
        .path = ffStrbufCreate(),
        .version = ffStrbufCreate(),
    };
    const char* error = ffDetectEditor(&result);

    if (error)
    {
        ffPrintError(FF_EDITOR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    ffPrintLogoAndKey(FF_EDITOR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
    if (result.exe)
    {
        fputs(result.exe, stdout);
        if (result.version.length)
            printf(" (%s)", result.version.chars);
    }
    else
    {
        ffStrbufWriteTo(&result.name, stdout);
    }
    putchar('\n');

    ffStrbufDestroy(&result.name);
    ffStrbufDestroy(&result.path);
    ffStrbufDestroy(&result.version);
}

bool ffParseEditorCommandOptions(FFEditorOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_EDITOR_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseEditorJsonObject(FFEditorOptions* options, yyjson_val* module)
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

        ffPrintError(FF_EDITOR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateEditorJsonConfig(FFEditorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyEditorOptions))) FFEditorOptions defaultOptions;
    ffInitEditorOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateEditorJsonResult(FF_MAYBE_UNUSED FFEditorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFEditorResult result = {
        .name = ffStrbufCreate(),
        .path = ffStrbufCreate(),
        .version = ffStrbufCreate(),
    };

    const char* error = ffDetectEditor(&result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &result.name);
    yyjson_mut_obj_add_strcpy(doc, obj, "exe", result.exe);
    yyjson_mut_obj_add_strbuf(doc, obj, "path", &result.path);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result.version);

    ffStrbufDestroy(&result.name);
    ffStrbufDestroy(&result.path);
    ffStrbufDestroy(&result.version);
}

void ffPrintEditorHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_EDITOR_MODULE_NAME, "{2} ({4})", FF_EDITOR_NUM_FORMAT_ARGS, ((const char* []) {
        "Name",
        "Exe name",
        "Full path",
        "Version",
    }));
}

void ffInitEditorOptions(FFEditorOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_EDITOR_MODULE_NAME,
        "Print information of the default editor ($VISUAL or $EDITOR)",
        ffParseEditorCommandOptions,
        ffParseEditorJsonObject,
        ffPrintEditor,
        ffGenerateEditorJsonResult,
        ffPrintEditorHelpFormat,
        ffGenerateEditorJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyEditorOptions(FFEditorOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
