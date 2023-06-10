#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/processing.h"
#include "modules/command/command.h"

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

    ffPrintLogoAndKey(instance, FF_COMMAND_MODULE_NAME, 0, &options->moduleArgs.key);
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

    if(strcasecmp(subKey, "shell") == 0)
    {
        ffOptionParseString(key, value, &options->shell);
        return true;
    }

    if(strcasecmp(subKey, "text") == 0)
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
            if(strcasecmp(key, "type") == 0)
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (strcasecmp(key, "shell") == 0)
            {
                ffStrbufSetS(&options.shell, yyjson_get_str(val));
                continue;
            }

            if (strcasecmp(key, "text") == 0)
            {
                ffStrbufSetS(&options.text, yyjson_get_str(val));
                continue;
            }

            ffPrintError(instance, FF_COMMAND_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintCommand(instance, &options);
}
