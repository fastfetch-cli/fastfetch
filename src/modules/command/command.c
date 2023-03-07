#include "fastfetch.h"

#include "common/printing.h"
#include "common/processing.h"
#include "modules/command/command.h"

#define FF_COMMAND_MODULE_NAME "Command"

void ffPrintCommand(FFinstance* instance, FFCommandOptions* options)
{
    FF_STRBUF_AUTO_DESTROY result;
    ffStrbufInit(&result);
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
