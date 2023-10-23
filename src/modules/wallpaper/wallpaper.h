#pragma once

#include "fastfetch.h"

#define FF_WALLPAPER_MODULE_NAME "Wallpaper"

void ffPrintWallpaper(FFWallpaperOptions* options);
void ffInitWallpaperOptions(FFWallpaperOptions* options);
void ffDestroyWallpaperOptions(FFWallpaperOptions* options);
