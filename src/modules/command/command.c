#include "common/printing.h"
#include "common/jsonconfig.h"
#include "modules/command/command.h"
#include "detection/command/command.h"

bool ffPrintCommand(FFCommandOptions* options)
{
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    const char* error = ffDetectCommand(options, &result);

    if (error)
    {
        ffPrintError(FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (!result.length)
    {
        ffPrintError(FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No result generated");
        return false;
    }

    if (options->splitLines)
    {
        uint8_t index = 0;
        char* line = NULL;
        size_t len = 0;
        while (ffStrbufGetline(&line, &len, &result))
        {
            if (options->moduleArgs.outputFormat.length == 0)
            {
                ffPrintLogoAndKey(FF_COMMAND_MODULE_NAME, ++index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
                puts(line);
            }
            else
            {
                FF_PRINT_FORMAT_CHECKED(FF_COMMAND_MODULE_NAME, ++index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
                    FF_FORMAT_ARG(line, "result")
                }));
            }
        }
    }
    else
    {
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

    return true;
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

        if (unsafe_yyjson_equals_str(key, "useStdErr"))
        {
            options->useStdErr = yyjson_get_bool(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "parallel"))
        {
            options->parallel = yyjson_get_bool(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "splitLines"))
        {
            options->splitLines = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateCommandJsonConfig(FFCommandOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_strbuf(doc, module, "shell", &options->shell);
    yyjson_mut_obj_add_strbuf(doc, module, "param", &options->param);
    yyjson_mut_obj_add_strbuf(doc, module, "text", &options->text);
    yyjson_mut_obj_add_bool(doc, module, "useStdErr", options->useStdErr);
    yyjson_mut_obj_add_bool(doc, module, "parallel", options->parallel);
    yyjson_mut_obj_add_bool(doc, module, "splitLines", options->splitLines);
}

bool ffGenerateCommandJsonResult(FF_MAYBE_UNUSED FFCommandOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    const char* error = ffDetectCommand(options, &result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    if(!result.length)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No result generated");
        return false;
    }

    if (options->splitLines)
    {
        yyjson_mut_val* jsonArray = yyjson_mut_obj_add_arr(doc, module, "result");
        char* line = NULL;
        size_t len = 0;
        while (ffStrbufGetline(&line, &len, &result))
            yyjson_mut_arr_add_strncpy(doc, jsonArray, line, len);
    }
    else
        yyjson_mut_obj_add_strbuf(doc, module, "result", &result);

    return true;
}

void ffInitCommandOptions(FFCommandOptions* options)
{
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
    options->useStdErr = false;
    options->parallel = true;
    options->splitLines = false;
}

void ffDestroyCommandOptions(FFCommandOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->shell);
    ffStrbufDestroy(&options->param);
    ffStrbufDestroy(&options->text);
}

FFModuleBaseInfo ffCommandModuleInfo = {
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
