#include "fastfetch.h"

void ffPrintWM(FFinstance* instance)
{
    FFstrbuf* prettyName;
    FFstrbuf* processName;
    FFstrbuf* error;

    ffCalculateWM(instance, &prettyName, &processName, &error);

    if(error->length > 0)
    {
        ffPrintError(instance, &instance->config.wmKey, "WM", error->chars);
        return;
    }

    if(instance->config.wmFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, &instance->config.wmKey, "WM");
        ffStrbufPutTo(prettyName, stdout);
    }
    else
    {
        ffPrintFormatString(instance, &instance->config.wmKey, "WM", &instance->config.wmFormat, 2,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, processName},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRBUF, prettyName}
        );
    }
}
