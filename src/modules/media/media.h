#pragma once

#include "option.h"

#define FF_MEDIA_MODULE_NAME "Media"

void ffPrintMedia(FFMediaOptions* options);
void ffInitMediaOptions(FFMediaOptions* options);
void ffDestroyMediaOptions(FFMediaOptions* options);

extern FFModuleBaseInfo ffMediaModuleInfo;
