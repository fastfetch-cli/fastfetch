#pragma once

#include "util/FFstrbuf.h"

typedef struct FFModuleArgs
{
    FFstrbuf key;
    FFstrbuf outputFormat;
    FFstrbuf errorFormat;
} FFModuleArgs;

const char* ffOptionTestPrefix(const char* argumentKey, const char* moduleName);
bool ffOptionParseModuleArgs(const char* argumentKey, const char* pkey, const char* value, FFModuleArgs* result);
void ffOptionParseString(const char* argumentKey, const char* value, FFstrbuf* buffer);
uint32_t ffOptionParseUInt32(const char* argumentKey, const char* value);
void ffOptionParseEnum(const char* argumentKey, const char* requestedKey, void* result, ...);
void ffOptionInitModuleArg(FFModuleArgs* args);
void ffOptionDestroyModuleArg(FFModuleArgs* args);
