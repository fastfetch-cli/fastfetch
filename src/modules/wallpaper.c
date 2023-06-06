#include "fastfetch.h"
#include "common/printing.h"
#include "detection/wallpaper/wallpaper.h"

#define FF_WALLPAPER_MODULE_NAME "Wallpaper"
#define FF_WALLPAPER_NUM_FORMAT_ARGS 1

void ffPrintWallpaper(FFinstance* instance)
{
    FF_STRBUF_AUTO_DESTROY wallpaper;
    ffStrbufInit(&wallpaper);
    const char* error = ffDetectWallpaper(instance, &wallpaper);

    if(error)
    {
        ffPrintError(instance, FF_WALLPAPER_MODULE_NAME, 0, &instance->config.wallpaper, "%s", error);
        return;
    }

    if(instance->config.wallpaper.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_WALLPAPER_MODULE_NAME, 0, &instance->config.wallpaper.key);
        ffStrbufPutTo(&wallpaper, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_WALLPAPER_MODULE_NAME, 0, &instance->config.wallpaper, FF_WALLPAPER_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &wallpaper}
        });
    }
}
