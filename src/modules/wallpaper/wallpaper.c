#include "common/printing.h"
#include "common/jsonconfig.h"
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

void ffParseWallpaperJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFWallpaperOptions __attribute__((__cleanup__(ffDestroyWallpaperOptions))) options;
    ffInitWallpaperOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(strcasecmp(key, "type") == 0)
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_WALLPAPER_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintWallpaper(instance, &options);
}
