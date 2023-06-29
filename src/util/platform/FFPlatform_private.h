#pragma once

#ifndef FF_INCLUDED_util_platform_FFPlatform_private
#define FF_INCLUDED_util_platform_FFPlatform_private

#include "FFPlatform.h"

void ffPlatformInitImpl(FFPlatform* platform);

void ffPlatformPathAddAbsolute(FFlist* dirs, const char* path);
void ffPlatformPathAddHome(FFlist* dirs, const FFPlatform* platform, const char* suffix);

#endif
