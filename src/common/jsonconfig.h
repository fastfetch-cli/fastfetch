#pragma once

#include "fastfetch.h"

bool ffJsonConfigParseModuleArgs(const char* key, yyjson_val* val, FFModuleArgs* moduleArgs);
const char* ffJsonConfigParseEnum(yyjson_val* val, int* result, FFKeyValuePair pairs[]);
void ffPrintJsonConfig();
const char* ffParseGeneralJsonConfig();
const char* ffParseDisplayJsonConfig();
const char* ffParseLibraryJsonConfig();
