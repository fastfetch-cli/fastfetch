#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/strutil.h"
#include "detection/wallpaper/wallpaper.h"
#include "modules/wallpaper/wallpaper.h"

bool ffPrintWallpaper(FFWallpaperOptions* options) {
    FF_STRBUF_AUTO_DESTROY fullpath = ffStrbufCreate();
    const char* error = ffDetectWallpaper(&fullpath);

    const uint32_t index = ffStrbufLastIndexC(&fullpath,
#ifndef _WIN32
                               '/'
#else
                               '\\'
#endif
                               ) +
        1;
    const char* filename = index >= fullpath.length
        ? fullpath.chars
        : fullpath.chars + index;

    if (error) {
        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Wallpaper), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (options->moduleArgs.outputFormat.length == 0) {
        ffPrintLogoAndKey(FF_MODULE_GET_DISPLAY_NAME(Wallpaper), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        puts(filename);
    } else {
        FF_PRINT_FORMAT_CHECKED(FF_MODULE_GET_DISPLAY_NAME(Wallpaper), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                                                                                                              FF_ARG(filename, "file-name"),
                                                                                                              FF_ARG(fullpath, "full-path"),
                                                                                                          }));
    }

    return true;
}

void ffParseWallpaperJsonObject(FFWallpaperOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        ffPrintError(FF_MODULE_GET_DISPLAY_NAME(Wallpaper), 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateWallpaperJsonConfig(FFWallpaperOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateWallpaperJsonResult([[maybe_unused]] FFWallpaperOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    FF_STRBUF_AUTO_DESTROY fullpath = ffStrbufCreate();
    const char* error = ffDetectWallpaper(&fullpath);
    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }
    yyjson_mut_obj_add_strbuf(doc, module, "result", &fullpath);

    return true;
}

void ffInitWallpaperOptions(FFWallpaperOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "󰸉");
}

void ffDestroyWallpaperOptions(FFWallpaperOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffWallpaperModuleInfo = {
    .name = "Wallpaper",
    .description = "Print the file path of the current wallpaper",
    .displayName = {
        .en = "Wallpaper",
        .de = "Hintergrundbild",
        .es = "Fondo de pantalla",
        .fr = "Fond d'écran",
        .it = "Sfondo",
        .ja = "壁紙",
        .ko = "배경화면",
        .pl = "Tapeta",
        .pt = "Papel de parede",
        .ru = "Обои",
        .zh_CN = "壁纸",
        .zh_TW = "桌布",
    },
    .initOptions = (void*) ffInitWallpaperOptions,
    .destroyOptions = (void*) ffDestroyWallpaperOptions,
    .parseJsonObject = (void*) ffParseWallpaperJsonObject,
    .printModule = (void*) ffPrintWallpaper,
    .generateJsonResult = (void*) ffGenerateWallpaperJsonResult,
    .generateJsonConfig = (void*) ffGenerateWallpaperJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        { "File name", "file-name" },
        { "Full path", "full-path" },
    })),
    .defaultOrder = 28,
};
