#include "fastfetch.h"

void ffPrintTerminal(FFinstance* instance)
{
    FFstrbuf* exeName;
    FFstrbuf* processName;
    FFstrbuf* error;

    ffCalculateTerminal(instance, &exeName, &processName, &error);

    if(error->length > 0)
    {
        ffPrintError(instance, &instance->config.terminalKey, "Terminal", error->chars);
        return;
    }

    FFstrbuf* name;

    if(ffStrbufStartsWith(exeName, processName))
        name = exeName;
    else
        name = processName;

    if(instance->config.terminalFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, &instance->config.terminalKey, "Terminal");
        ffStrbufPutTo(name, stdout);
    }
    else
    {
        ffPrintFormatString(instance, &instance->config.terminalKey, "Terminal", &instance->config.terminalFormat, 3,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, exeName},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, processName},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, name}
        );
    }
}
