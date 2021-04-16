#include "fastfetch.h"

#define FF_WM_MODULE_NAME "WM"
#define FF_WM_NUM_FORMAT_ARGS 2

void ffPrintWM(FFinstance* instance)
{
    ffCalculateWM(instance);

    if(instance->state.wm.error.length > 0)
    {
        ffPrintError(instance, FF_WM_MODULE_NAME, 0, &instance->config.wmKey, &instance->config.wmFormat, FF_WM_NUM_FORMAT_ARGS, instance->state.wm.error.chars);
        return;
    }

    if(instance->state.wm.prettyName.length == 0 && instance->state.wm.processName.length == 0)
    {
        ffPrintError(instance, FF_WM_MODULE_NAME, 0, &instance->config.wmKey, &instance->config.wmFormat, FF_WM_NUM_FORMAT_ARGS, "No wm name provided");
        return;
    }

    if(instance->config.wmFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, "WM", 0, &instance->config.wmKey);
        if(instance->state.wm.prettyName.length > 0)
            ffStrbufPutTo(&instance->state.wm.prettyName, stdout);
        else
            ffStrbufPutTo(&instance->state.wm.processName, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_WM_MODULE_NAME, 0, &instance->config.wmKey, &instance->config.wmFormat, NULL, FF_WM_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.wm.processName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &instance->state.wm.prettyName}
        });
    }
}
