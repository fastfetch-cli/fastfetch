#pragma once

#include "fastfetch.h"

#define FF_SWAP_MODULE_NAME "Swap"

void ffPrintSwap(FFinstance* instance, FFSwapOptions* options);
void ffInitSwapOptions(FFSwapOptions* options);
bool ffParseSwapCommandOptions(FFSwapOptions* options, const char* key, const char* value);
void ffDestroySwapOptions(FFSwapOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseSwapJsonObject(FFinstance* instance, json_object* module);
#endif
