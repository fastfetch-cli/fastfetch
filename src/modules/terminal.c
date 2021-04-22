#include "fastfetch.h"

#define FF_TERMINAL_MODULE_NAME "Terminal"
#define FF_TERMINAL_NUM_FORMAT_ARGS 3

void ffPrintTerminal(FFinstance* instance)
{
    const FFTerminalResult* result = ffCalculateTerminal(instance);

    if(result->error.length > 0)
    {
        ffPrintError(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, FF_TERMINAL_NUM_FORMAT_ARGS, result->error.chars);
        return;
    }

    if(result->exeName.length == 0 && result->processName.length == 0)
    {
        ffPrintError(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, FF_TERMINAL_NUM_FORMAT_ARGS, "Terminal names not set");
        return;
    }

    const FFstrbuf* name;

    if(ffStrbufStartsWith(&result->exeName, &result->processName))
        name = &result->exeName;
    else
        name = &result->processName;

    if(instance->config.terminalFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey);
        ffStrbufPutTo(name, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, NULL, FF_TERMINAL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->exeName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->processName},
            {FF_FORMAT_ARG_TYPE_STRBUF, name}
        });
    }
}
