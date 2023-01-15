#pragma once

#ifndef FF_INCLUDED_util_platform_FFPlatform_private
#define FF_INCLUDED_util_platform_FFPlatform_private

#include "FFPlatform.h"

void ffPlatformInitÎmpl(FFPlatform* platform);

#define FF_PLATFORM_PATH_UNIQUE(list, element) \
    if(ffListFirstIndexComp(list, element, (bool(*)(const void*, const void*))ffStrbufEqual) < list->length - 1) \
    { \
        ffStrbufDestroy(ffListGet(list, list->length - 1)); \
        --list->length; \
    }

void ffPlatformPathAddAbsolute(FFlist* dirs, const char* path);
void ffPlatformPathAddHome(FFlist* dirs, const FFPlatform* platform, const char* suffix);
void ffPlatformPathAddEnv(FFlist* dirs, const char* env);

#endif
