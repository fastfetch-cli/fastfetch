#include "fastfetch.h"

#define FF_TERMINAL_MODULE_NAME "Terminal"
#define FF_TERMINAL_NUM_FORMAT_ARGS 3

void ffPrintTerminal(FFinstance* instance)
{
    FFstrbuf* exeName;
    FFstrbuf* processName;
    FFstrbuf* error;

    ffCalculateTerminal(instance, &exeName, &processName, &error);

    if(error->length > 0)
    {
        ffPrintError(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, FF_TERMINAL_NUM_FORMAT_ARGS, error->chars);
        return;
    }

    FFstrbuf* name;

    if(ffStrbufStartsWith(exeName, processName))
        name = exeName;
    else
        name = processName;

    if(instance->config.terminalFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey);
        ffStrbufPutTo(name, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_TERMINAL_MODULE_NAME, 0, &instance->config.terminalKey, &instance->config.terminalFormat, NULL, FF_TERMINAL_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, exeName},
            {FF_FORMAT_ARG_TYPE_STRBUF, processName},
            {FF_FORMAT_ARG_TYPE_STRBUF, name}
        });
    }
}
