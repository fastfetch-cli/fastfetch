#pragma once

#include "fastfetch.h"

#define FF_NATIVERESOLUTION_MODULE_NAME "NativeResolution"

void ffPrintNativeResolution(FFNativeResolutionOptions* options);
void ffInitNativeResolutionOptions(FFNativeResolutionOptions* options);
bool ffParseNativeResolutionCommandOptions(FFNativeResolutionOptions* options, const char* key, const char* value);
void ffDestroyNativeResolutionOptions(FFNativeResolutionOptions* options);
void ffParseNativeResolutionJsonObject(yyjson_val* module);
