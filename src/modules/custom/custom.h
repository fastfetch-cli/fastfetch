#pragma once

#include "fastfetch.h"

#define FF_CUSTOM_MODULE_NAME "Custom"

void ffPrintCustom(FFCustomOptions* options);
void ffInitCustomOptions(FFCustomOptions* options);
void ffDestroyCustomOptions(FFCustomOptions* options);
