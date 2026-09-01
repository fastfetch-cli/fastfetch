#pragma once

#include "option.h"

bool ffPrintWallpaper(FFWallpaperOptions* options);
void ffInitWallpaperOptions(FFWallpaperOptions* options);
void ffDestroyWallpaperOptions(FFWallpaperOptions* options);

extern FFModuleBaseInfo ffWallpaperModuleInfo;
