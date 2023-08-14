#pragma once

#include "util/FFstrbuf.h"

typedef struct FFModuleArgs
{
    FFstrbuf key;
    FFstrbuf keyColor;
    FFstrbuf outputFormat;
} FFModuleArgs;

typedef struct FFKeyValuePair
{
    const char* key;
    int value;
} FFKeyValuePair;

const char* ffOptionTestPrefix(const char* argumentKey, const char* moduleName);
bool ffOptionParseModuleArgs(const char* argumentKey, const char* pkey, const char* value, FFModuleArgs* result);
void ffOptionParseString(const char* argumentKey, const char* value, FFstrbuf* buffer);
FF_C_NODISCARD uint32_t ffOptionParseUInt32(const char* argumentKey, const char* value);
FF_C_NODISCARD int32_t ffOptionParseInt32(const char* argumentKey, const char* value);
FF_C_NODISCARD int ffOptionParseEnum(const char* argumentKey, const char* requestedKey, FFKeyValuePair pairs[]);
FF_C_NODISCARD bool ffOptionParseBoolean(const char* str);
void ffOptionParseColor(const char* value, FFstrbuf* buffer);
void ffOptionInitModuleArg(FFModuleArgs* args);
void ffOptionDestroyModuleArg(FFModuleArgs* args);
