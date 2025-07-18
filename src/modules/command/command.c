#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/processing.h"
#include "modules/command/command.h"
#include "util/stringUtils.h"

void ffPrintCommand(FFCommandOptions* options)
{
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    const char* error = ffProcessAppendStdOut(&result, options->param.length ? (char* const[]){
        options->shell.chars,
        options->param.chars,
        options->text.chars,
        NULL
    } : (char* const[]){
        options->shell.chars,
        options->text.chars,
        NULL
    });

    if(error)
    {
        ffPrintError(FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No result printed");
        return;
    }

    if (options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&result, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(result, "result")
        }));
    }
}

bool ffParseCommandCommandOptions(FFCommandOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_COMMAND_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if(ffStrEqualsIgnCase(subKey, "shell"))
    {
        ffOptionParseString(key, value, &options->shell);
        return true;
    }

    if(ffStrEqualsIgnCase(subKey, "param"))
    {
        ffOptionParseString(key, value, &options->param);
        return true;
    }

    if(ffStrEqualsIgnCase(subKey, "text"))
    {
        ffOptionParseString(key, value, &options->text);
        return true;
    }

    return false;
}

void ffParseCommandJsonObject(FFCommandOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type") || ffStrEqualsIgnCase(key, "condition"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffStrEqualsIgnCase(key, "shell"))
        {
            ffStrbufSetS(&options->shell, yyjson_get_str(val));
            continue;
        }

        if (ffStrEqualsIgnCase(key, "param"))
        {
            ffStrbufSetS(&options->param, yyjson_get_str(val));
            continue;
        }

        if (ffStrEqualsIgnCase(key, "text"))
        {
            ffStrbufSetS(&options->text, yyjson_get_str(val));
            continue;
        }

        ffPrintError(FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateCommandJsonConfig(FFCommandOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyCommandOptions))) FFCommandOptions defaultOptions;
    ffInitCommandOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (!ffStrbufEqual(&defaultOptions.shell, &options->shell))
        yyjson_mut_obj_add_strbuf(doc, module, "shell", &options->shell);

    if (!ffStrbufEqual(&defaultOptions.param, &options->param))
        yyjson_mut_obj_add_strbuf(doc, module, "param", &options->param);

    if (!ffStrbufEqual(&defaultOptions.text, &options->text))
        yyjson_mut_obj_add_strbuf(doc, module, "text", &options->text);
}

void ffGenerateCommandJsonResult(FF_MAYBE_UNUSED FFCommandOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    const char* error = ffProcessAppendStdOut(&result, options->param.length ? (char* const[]){
        options->shell.chars,
        options->param.chars,
        options->text.chars,
        NULL
    } : (char* const[]){
        options->shell.chars,
        options->text.chars,
        NULL
    });

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    if(!result.length)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No result printed");
        return;
    }

    yyjson_mut_obj_add_strbuf(doc, module, "result", &result);
}

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_COMMAND_MODULE_NAME,
    .description = "Run custom shell scripts",
    .parseCommandOptions = (void*) ffParseCommandCommandOptions,
    .parseJsonObject = (void*) ffParseCommandJsonObject,
    .printModule = (void*) ffPrintCommand,
    .generateJsonResult = (void*) ffGenerateCommandJsonResult,
    .generateJsonConfig = (void*) ffGenerateCommandJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Command result", "result"},
    }))
};

void ffInitCommandOptions(FFCommandOptions* options)
{
    options->moduleInfo = ffModuleInfo;
    ffOptionInitModuleArg(&options->moduleArgs, "");

    ffStrbufInitStatic(&options->shell,
        #ifdef _WIN32
        "cmd.exe"
        #else
        "/bin/sh"
        #endif
    );
    ffStrbufInitStatic(&options->param,
        #ifdef _WIN32
        "/c"
        #else
        "-c"
        #endif
    );
    ffStrbufInit(&options->text);
}

void ffDestroyCommandOptions(FFCommandOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->shell);
    ffStrbufDestroy(&options->param);
    ffStrbufDestroy(&options->text);
}
