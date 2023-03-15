#pragma once

#ifdef FF_HAVE_JSONC

#include "common/json.h"

bool ffJsonConfigParseModuleArgs(const char* key, json_object* val, FFModuleArgs* moduleArgs);
const char* ffJsonConfigParseEnum(json_object* val, int* result, FFKeyValuePair pairs[]);

#endif
