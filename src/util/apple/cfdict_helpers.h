#pragma once

#ifndef FASTFETCH_INCLUDED_cfdict_helpers
#define FASTFETCH_INCLUDED_cfdict_helpers

#include "fastfetch.h"
#include <CoreFoundation/CoreFoundation.h>

bool ffCfDictGetString(CFDictionaryRef dict, CFStringRef key, FFstrbuf* result);
bool ffCfDictGetBool(CFDictionaryRef dict, CFStringRef key, bool* result);
bool ffCfDictGetInt(CFDictionaryRef dict, CFStringRef key, int* result);

#endif
