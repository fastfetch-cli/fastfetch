#include "fastfetch.h"

#include "common/printing.h"
#include "common/processing.h"

#define FF_COMMAND_MODULE_NAME "Command"

static void destroyModuleArg(FFModuleArgs* args)
{
    ffStrbufDestroy(&args->key);
    ffStrbufDestroy(&args->outputFormat);
    ffStrbufDestroy(&args->errorFormat);
}

void ffPrintCommand(FFinstance* instance)
{
    FFModuleArgs __attribute__((__cleanup__(destroyModuleArg))) arg;

    if(!ffListShift(&instance->config.commandKeys, &arg.key))
        ffStrbufInitS(&arg.key, FF_COMMAND_MODULE_NAME);
    ffStrbufInit(&arg.outputFormat);
    ffStrbufInit(&arg.errorFormat);

    FF_STRBUF_AUTO_DESTROY text;
    ffStrbufInit(&text);
    if(!ffListShift(&instance->config.commandTexts, &text))
    {
        ffPrintError(instance, FF_COMMAND_MODULE_NAME, 0, &arg, "No command text left");
        ffStrbufDestroy(&arg.key);
        return;
    }

    FF_STRBUF_AUTO_DESTROY result;
    ffStrbufInit(&result);
    const char* error = ffProcessAppendStdOut(&result, (char* const[]){
        instance->config.commandShell.chars,
        "-c",
        text.chars
    });

    if(error)
    {
        ffPrintError(instance, FF_COMMAND_MODULE_NAME, 0, &arg, "%s", error);
        ffStrbufDestroy(&arg.key);
        return;
    }

    if(!result.length)
    {
        ffPrintError(instance, FF_COMMAND_MODULE_NAME, 0, &arg, "No result printed");
        ffStrbufDestroy(&arg.key);
        return;
    }

    ffPrintLogoAndKey(instance, FF_COMMAND_MODULE_NAME, 0, &arg.key);
    puts(result.chars);
    ffStrbufDestroy(&arg.key);
}
