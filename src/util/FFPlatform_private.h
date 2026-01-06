#pragma once

#include "FFPlatform.h"

void ffPlatformInitImpl(FFPlatform* platform);

void ffPlatformPathAddAbsolute(FFlist* dirs, const char* path);
void ffPlatformPathAddHome(FFlist* dirs, const FFPlatform* platform, const char* suffix);
