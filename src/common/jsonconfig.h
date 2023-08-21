#pragma once

#include "fastfetch.h"

bool ffJsonConfigParseModuleArgs(const char* key, yyjson_val* val, FFModuleArgs* moduleArgs);
const char* ffJsonConfigParseEnum(yyjson_val* val, int* result, FFKeyValuePair pairs[]);
void ffPrintJsonConfig(bool prepare);
const char* ffParseGeneralJsonConfig(void);
const char* ffParseDisplayJsonConfig(void);
const char* ffParseLibraryJsonConfig(void);
