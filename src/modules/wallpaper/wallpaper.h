#pragma once

#include "fastfetch.h"

#define FF_WALLPAPER_MODULE_NAME "Wallpaper"

void ffPrintWallpaper(FFinstance* instance, FFWallpaperOptions* options);
void ffInitWallpaperOptions(FFWallpaperOptions* options);
bool ffParseWallpaperCommandOptions(FFWallpaperOptions* options, const char* key, const char* value);
void ffDestroyWallpaperOptions(FFWallpaperOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseWallpaperJsonObject(FFinstance* instance, json_object* module);
#endif
