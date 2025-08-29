#pragma once

#include "option.h"

#define FF_WALLPAPER_MODULE_NAME "Wallpaper"

bool ffPrintWallpaper(FFWallpaperOptions* options);
void ffInitWallpaperOptions(FFWallpaperOptions* options);
void ffDestroyWallpaperOptions(FFWallpaperOptions* options);

extern FFModuleBaseInfo ffWallpaperModuleInfo;
