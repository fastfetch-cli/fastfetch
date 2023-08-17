#pragma once

#include "util/FFstrbuf.h"
#include "3rdparty/yyjson/yyjson.h"

// Must be the first field of FFModuleOptions
typedef struct FFModuleBaseInfo
{
    const char* name;
    bool (*parseCommandOptions)(void* options, const char* key, const char* value);
    void (*parseJsonObject)(void* options, yyjson_val *module);
    void (*printModule)(void* options);
} FFModuleBaseInfo;

static inline void ffOptionInitModuleBaseInfo(
    FFModuleBaseInfo* baseInfo,
    const char* name,
    void* parseCommandOptions, // bool (*const parseCommandOptions)(void* options, const char* key, const char* value)
    void* parseJsonObject, // void (*const parseJsonObject)(void* options, yyjson_val *module)
    void* printModule // void (*const printModule)(void* options)
)
{
    baseInfo->name = name;
    baseInfo->parseCommandOptions = parseCommandOptions;
    baseInfo->parseJsonObject = parseJsonObject;
    baseInfo->printModule = printModule;
}

typedef struct FFModuleArgs
{
    FFstrbuf key;
    FFstrbuf keyColor;
    FFstrbuf outputFormat;
    uint32_t keyWidth;
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
