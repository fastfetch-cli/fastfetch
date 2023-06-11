#pragma once

#include "fastfetch.h"

bool ffJsonConfigParseModuleArgs(const char* key, yyjson_val* val, FFModuleArgs* moduleArgs);
const char* ffJsonConfigParseEnum(yyjson_val* val, int* result, FFKeyValuePair pairs[]);
void ffPrintJsonConfig(FFinstance* instance);
const char* ffParseGeneralJsonConfig(FFinstance* instance);
const char* ffParseDisplayJsonConfig(FFinstance* instance);
const char* ffParseLibraryJsonConfig(FFinstance* instance);
