#include "fastfetch.h"

#include "common/printing.h"
#include "common/processing.h"
#include "util/textModifier.h"

#define FF_COMMAND_MODULE_NAME "Command"

static void printError(FFinstance* instance, const char* key, const char* error)
{
    ffPrintLogoAndKey(instance, key, 0, NULL);

    if(instance->config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_ERROR, stdout);

    fputs(error, stdout);

    if(!instance->config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    putchar('\n');
}

void ffPrintCommand(FFinstance* instance)
{
    FF_STRBUF_AUTO_DESTROY key;
    ffStrbufInit(&key);

    if(!ffListShift(&instance->config.commandKeys, &key))
        ffStrbufInitS(&key, FF_COMMAND_MODULE_NAME);

    FF_STRBUF_AUTO_DESTROY text;
    ffStrbufInit(&text);
    if(!ffListShift(&instance->config.commandTexts, &text))
    {
        printError(instance, key.chars, "No command text left");
        return;
    }

    FF_STRBUF_AUTO_DESTROY result;
    ffStrbufInit(&result);
    const char* error = ffProcessAppendStdOut(&result, (char* const[]){
        instance->config.commandShell.chars,
        #ifdef _WIN32
        "/c",
        #else
        "-c",
        #endif
        text.chars,
        NULL
    });

    if(error)
    {
        printError(instance, key.chars, error);
        return;
    }

    if(!result.length)
    {
        printError(instance, key.chars, "No result printed");
        return;
    }

    ffPrintLogoAndKey(instance, key.chars, 0, NULL);
    puts(result.chars);
}
