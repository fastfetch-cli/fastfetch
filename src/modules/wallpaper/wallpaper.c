#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/wallpaper/wallpaper.h"
#include "modules/wallpaper/wallpaper.h"
#include "util/stringUtils.h"

#define FF_WALLPAPER_NUM_FORMAT_ARGS 2

void ffPrintWallpaper(FFWallpaperOptions* options)
{
    FF_STRBUF_AUTO_DESTROY fullpath = ffStrbufCreate();
    const char* error = ffDetectWallpaper(&fullpath);

    const uint32_t index = ffStrbufLastIndexC(&fullpath,
        #ifndef _WIN32
        '/'
        #else
        '\\'
        #endif
    ) + 1;
    const char* filename = index >= fullpath.length
        ? fullpath.chars
        : fullpath.chars + index;

    if(error)
    {
        ffPrintError(FF_WALLPAPER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_WALLPAPER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        puts(filename);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_WALLPAPER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_WALLPAPER_NUM_FORMAT_ARGS, ((FFformatarg[]){
            FF_FORMAT_ARG(filename, "file-name"),
            FF_FORMAT_ARG(fullpath, "full-path"),
        }));
    }
}

bool ffParseWallpaperCommandOptions(FFWallpaperOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_WALLPAPER_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseWallpaperJsonObject(FFWallpaperOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_WALLPAPER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateWallpaperJsonConfig(FFWallpaperOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyWallpaperOptions))) FFWallpaperOptions defaultOptions;
    ffInitWallpaperOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateWallpaperJsonResult(FF_MAYBE_UNUSED FFWallpaperOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_STRBUF_AUTO_DESTROY fullpath = ffStrbufCreate();
    const char* error = ffDetectWallpaper(&fullpath);
    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }
    yyjson_mut_obj_add_strbuf(doc, module, "result", &fullpath);
}

void ffPrintWallpaperHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_WALLPAPER_MODULE_NAME, "{1}", FF_WALLPAPER_NUM_FORMAT_ARGS, ((const char* []) {
        "File name - file-name",
        "Full path - full-path",
    }));
}

void ffInitWallpaperOptions(FFWallpaperOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_WALLPAPER_MODULE_NAME,
        "Print image file path of current wallpaper",
        ffParseWallpaperCommandOptions,
        ffParseWallpaperJsonObject,
        ffPrintWallpaper,
        ffGenerateWallpaperJsonResult,
        ffPrintWallpaperHelpFormat,
        ffGenerateWallpaperJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "󰸉");
}

void ffDestroyWallpaperOptions(FFWallpaperOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
