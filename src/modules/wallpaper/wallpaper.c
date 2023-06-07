#include "fastfetch.h"
#include "common/printing.h"
#include "detection/wallpaper/wallpaper.h"
#include "modules/wallpaper/wallpaper.h"

#define FF_WALLPAPER_NUM_FORMAT_ARGS 1

void ffPrintWallpaper(FFinstance* instance, FFWallpaperOptions* options)
{
    FF_STRBUF_AUTO_DESTROY wallpaper = ffStrbufCreate();
    const char* error = ffDetectWallpaper(instance, &wallpaper);

    if(error)
    {
        ffPrintError(instance, FF_WALLPAPER_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_WALLPAPER_MODULE_NAME, 0, &options->moduleArgs.key);
        ffStrbufPutTo(&wallpaper, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_WALLPAPER_MODULE_NAME, 0, &options->moduleArgs, FF_WALLPAPER_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &wallpaper}
        });
    }
}

void ffInitWallpaperOptions(FFWallpaperOptions* options)
{
    options->moduleName = FF_WALLPAPER_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseWallpaperCommandOptions(FFWallpaperOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_WALLPAPER_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyWallpaperOptions(FFWallpaperOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
void ffParseWallpaperJsonObject(FFinstance* instance, json_object* module)
{
    FFWallpaperOptions __attribute__((__cleanup__(ffDestroyWallpaperOptions))) options;
    ffInitWallpaperOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_WALLPAPER_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintWallpaper(instance, &options);
}
#endif
