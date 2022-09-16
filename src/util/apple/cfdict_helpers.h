#pragma once

#ifndef FASTFETCH_INCLUDED_cfdict_helpers
#define FASTFETCH_INCLUDED_cfdict_helpers

#include "fastfetch.h"
#include <CoreFoundation/CoreFoundation.h>

const void* ffCfDictGetValue(CFMutableDictionaryRef dict, const char* str);
bool ffCfDictGetString(CFMutableDictionaryRef dict, const char* key, FFstrbuf* result);
bool ffCfDictGetBool(CFMutableDictionaryRef dict, const char* key, bool* result);
bool ffCfDictGetInt(CFMutableDictionaryRef dict, const char* key, int* result);

#endif
