#pragma once

#include "fastfetch.h"

#define FF_DISPLAY_MODULE_NAME "Display"

void ffPrintDisplay(FFDisplayOptions* options);
void ffInitDisplayOptions(FFDisplayOptions* options);
void ffDestroyDisplayOptions(FFDisplayOptions* options);
