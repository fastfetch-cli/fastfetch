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

void ffParseCommandJsonObject(FFCommandOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "shell"))
        {
            ffStrbufSetJsonVal(&options->shell, val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "param"))
        {
            ffStrbufSetJsonVal(&options->param, val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "text"))
        {
            ffStrbufSetJsonVal(&options->text, val);
            continue;
        }

        ffPrintError(FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
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
    .initOptions = (void*) ffInitCommandOptions,
    .destroyOptions = (void*) ffDestroyCommandOptions,
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
