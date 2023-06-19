#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/processing.h"
#include "modules/command/command.h"
#include "util/stringUtils.h"

void ffPrintCommand(FFinstance* instance, FFCommandOptions* options)
{
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    const char* error = ffProcessAppendStdOut(&result, (char* const[]){
        options->shell.chars,
        #ifdef _WIN32
        "/c",
        #else
        "-c",
        #endif
        options->text.chars,
        NULL
    });

    if(error)
    {
        ffPrintError(instance, FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(!result.length)
    {
        ffPrintError(instance, FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs, "No result printed");
        return;
    }

    ffPrintLogoAndKey(instance, FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);
    ffStrbufPutTo(&result, stdout);
}

void ffInitCommandOptions(FFCommandOptions* options)
{
    options->moduleName = FF_COMMAND_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);

    ffStrbufInitS(&options->shell,
        #ifdef _WIN32
        "cmd"
        #elif defined(__FreeBSD__)
        "csh"
        #else
        "bash"
        #endif
    );

    ffStrbufInit(&options->text);
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

    if(ffStrEqualsIgnCase(subKey, "text"))
    {
        ffOptionParseString(key, value, &options->text);
        return true;
    }

    return false;
}

void ffDestroyCommandOptions(FFCommandOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->shell);
    ffStrbufDestroy(&options->text);
}

void ffParseCommandJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFCommandOptions __attribute__((__cleanup__(ffDestroyCommandOptions))) options;
    ffInitCommandOptions(&options);

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

            if (ffStrEqualsIgnCase(key, "shell"))
            {
                ffStrbufSetS(&options.shell, yyjson_get_str(val));
                continue;
            }

            if (ffStrEqualsIgnCase(key, "text"))
            {
                ffStrbufSetS(&options.text, yyjson_get_str(val));
                continue;
            }

            ffPrintError(instance, FF_COMMAND_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintCommand(instance, &options);
}
