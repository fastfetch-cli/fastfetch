#pragma once

#include "fastfetch.h"

#define FF_WALLPAPER_MODULE_NAME "Wallpaper"

void ffPrintWallpaper(FFWallpaperOptions* options);
void ffInitWallpaperOptions(FFWallpaperOptions* options);
bool ffParseWallpaperCommandOptions(FFWallpaperOptions* options, const char* key, const char* value);
void ffDestroyWallpaperOptions(FFWallpaperOptions* options);
void ffParseWallpaperJsonObject(FFWallpaperOptions* options, yyjson_val* module);
void ffGenerateWallpaperJson(FFWallpaperOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintWallpaperHelpFormat(void);
