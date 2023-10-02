#pragma once

#include "fastfetch.h"

#define FF_SWAP_MODULE_NAME "Swap"

void ffPrintSwap(FFSwapOptions* options);
void ffInitSwapOptions(FFSwapOptions* options);
bool ffParseSwapCommandOptions(FFSwapOptions* options, const char* key, const char* value);
void ffDestroySwapOptions(FFSwapOptions* options);
void ffParseSwapJsonObject(FFSwapOptions* options, yyjson_val* module);
void ffGenerateSwapJson(FFSwapOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
