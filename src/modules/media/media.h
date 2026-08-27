#pragma once

#include "option.h"

bool ffPrintMedia(FFMediaOptions* options);
void ffInitMediaOptions(FFMediaOptions* options);
void ffDestroyMediaOptions(FFMediaOptions* options);

extern FFModuleBaseInfo ffMediaModuleInfo;
