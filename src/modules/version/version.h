#pragma once

#include "fastfetch.h"

#define FF_VERSION_MODULE_NAME "Version"

void ffPrintVersion(FFVersionOptions* options);
void ffInitVersionOptions(FFVersionOptions* options);
void ffDestroyVersionOptions(FFVersionOptions* options);
