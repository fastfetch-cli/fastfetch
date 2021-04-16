#include "fastfetch.h"

#define FF_TERMINAL_MODULE_NAME "Terminal"
#define FF_TERMINAL_NUM_FORMAT_ARGS 3

void ffPrintTerminal(FFinstance* instance)
{
    ffCalculateTerminal(instance);

    if(instance->state.terminal.error.length > 0)
    {
        ffPrintError(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, FF_TERMINAL_NUM_FORMAT_ARGS, instance->state.terminal.error.chars);
        return;
    }

    if(instance->state.terminal.exeName.length == 0 && instance->state.terminal.processName.length == 0)
    {
        ffPrintError(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, FF_TERMINAL_NUM_FORMAT_ARGS, "Terminal names not set");
        return;
    }

    FFstrbuf* name;

    if(ffStrbufStartsWith(&instance->state.terminal.exeName, &instance->state.terminal.processName))
        name = &instance->state.terminal.exeName;
    else
        name = &instance->state.terminal.processName;

    if(instance->config.terminalFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey);
        ffStrbufPutTo(name, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, NULL, FF_TERMINAL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.terminal.exeName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.terminal.processName},
            {FF_FORMAT_ARG_TYPE_STRBUF, name}
        });
    }
}
