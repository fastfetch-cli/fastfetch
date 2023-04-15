#include "fastfetch.h"
#include "common/printing.h"
#include "detection/icons/icons.h"

#define FF_ICONS_MODULE_NAME "Icons"
#define FF_ICONS_NUM_FORMAT_ARGS 1

void ffPrintIcons(FFinstance* instance)
{
    FF_STRBUF_AUTO_DESTROY icons = ffStrbufCreate();
    const char* error = ffDetectIcons(instance, &icons);

    if(error)
    {
        ffPrintError(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.icons, "%s", error);
        return;
    }

    if(instance->config.icons.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.icons.key);
        ffStrbufPutTo(&icons, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_ICONS_MODULE_NAME, 0, &instance->config.icons, FF_ICONS_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &icons}
        });
    }
}
