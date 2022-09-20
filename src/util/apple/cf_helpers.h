#pragma once

#ifndef FASTFETCH_INCLUDED_cf_helpers
#define FASTFETCH_INCLUDED_cf_helpers

#include "fastfetch.h"
#include <CoreFoundation/CoreFoundation.h>

//Return error info if failed, NULL otherwise
const char* ffCfStrGetString(CFStringRef str, FFstrbuf* result);
const char* ffCfDictGetString(CFDictionaryRef dict, CFStringRef key, FFstrbuf* result);
const char* ffCfDictGetBool(CFDictionaryRef dict, CFStringRef key, bool* result);
const char* ffCfDictGetInt(CFDictionaryRef dict, CFStringRef key, int* result);

#endif
