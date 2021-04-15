#include "fastfetch.h"

#define FF_WM_MODULE_NAME "WM"
#define FF_WM_NUM_FORMAT_ARGS 2

void ffPrintWM(FFinstance* instance)
{
    FFstrbuf* prettyName;
    FFstrbuf* processName;
    FFstrbuf* error;

    ffCalculateWM(instance, &prettyName, &processName, &error);

    if(error->length > 0)
    {
        ffPrintError(instance, FF_WM_MODULE_NAME, 0, &instance->config.wmKey, &instance->config.wmFormat, FF_WM_NUM_FORMAT_ARGS, error->chars);
        return;
    }

    if(instance->config.wmFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, "WM", 0, &instance->config.wmKey);
        ffStrbufPutTo(prettyName, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_WM_MODULE_NAME, 0, &instance->config.wmKey, &instance->config.wmFormat, NULL, FF_WM_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, processName},
            {FF_FORMAT_ARG_TYPE_STRBUF, prettyName}
        });
    }
}
