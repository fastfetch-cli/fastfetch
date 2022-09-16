#pragma once

#ifndef FASTFETCH_INCLUDED_cfdict_helpers
#define FASTFETCH_INCLUDED_cfdict_helpers

#include "fastfetch.h"
#include <CoreFoundation/CoreFoundation.h>

bool ffCfDictGetString(CFMutableDictionaryRef dict, CFStringRef key, FFstrbuf* result);
bool ffCfDictGetBool(CFMutableDictionaryRef dict, CFStringRef key, bool* result);
bool ffCfDictGetInt(CFMutableDictionaryRef dict, CFStringRef key, int* result);

#endif
