#pragma once

#include "util/FFstrbuf.h"

struct yyjson_val;
struct yyjson_mut_doc;
struct yyjson_mut_val;

// Must be the first field of FFModuleOptions
typedef struct FFModuleBaseInfo
{
    const char* name;
    const char* description;
    // A dirty polymorphic implementation in C.
    // This is UB, because `void*` is not compatible with `FF*Options*`.
    // However we can't do it better unless we move to C++, so that `option` becomes a `this` pointer
    // https://stackoverflow.com/questions/559581/casting-a-function-pointer-to-another-type
    bool (*parseCommandOptions)(void* options, const char* key, const char* value);
    void (*parseJsonObject)(void* options, struct yyjson_val *module);
    void (*printModule)(void* options);
    void (*generateJsonResult)(void* options, struct yyjson_mut_doc* doc, struct yyjson_mut_val* module);
    void (*printHelpFormat)(void);
    void (*generateJsonConfig)(void* options, struct yyjson_mut_doc* doc, struct yyjson_mut_val* obj);
} FFModuleBaseInfo;

static inline void ffOptionInitModuleBaseInfo(
    FFModuleBaseInfo* baseInfo,
    const char* name,
    const char* description,
    void* parseCommandOptions, // bool (*const parseCommandOptions)(void* options, const char* key, const char* value)
    void* parseJsonObject, // void (*const parseJsonObject)(void* options, yyjson_val *module)
    void* printModule, // void (*const printModule)(void* options)
    void* generateJsonResult, // void (*const generateJsonResult)(void* options, yyjson_mut_doc* doc, yyjson_mut_val* obj)
    void (*printHelpFormat)(void),
    void* generateJsonConfig // void (*const generateJsonConfig)(void* options, yyjson_mut_doc* doc, yyjson_mut_val* obj)
)
{
    baseInfo->name = name;
    baseInfo->description = description;
    baseInfo->parseCommandOptions = (__typeof__(baseInfo->parseCommandOptions)) parseCommandOptions;
    baseInfo->parseJsonObject = (__typeof__(baseInfo->parseJsonObject)) parseJsonObject;
    baseInfo->printModule = (__typeof__(baseInfo->printModule)) printModule;
    baseInfo->generateJsonResult = (__typeof__(baseInfo->generateJsonResult)) generateJsonResult;
    baseInfo->printHelpFormat = printHelpFormat;
    baseInfo->generateJsonConfig = (__typeof__(baseInfo->generateJsonConfig)) generateJsonConfig;
}

typedef enum FFModuleKeyType
{
    FF_MODULE_KEY_TYPE_NONE = 0,
    FF_MODULE_KEY_TYPE_STRING = 1 << 0,
    FF_MODULE_KEY_TYPE_ICON = 1 << 1,
    FF_MODULE_KEY_TYPE_BOTH = FF_MODULE_KEY_TYPE_STRING | FF_MODULE_KEY_TYPE_ICON,
} FFModuleKeyType;

typedef struct FFModuleArgs
{
    FFstrbuf key;
    FFstrbuf keyColor;
    FFstrbuf keyIcon;
    FFstrbuf outputFormat;
    FFstrbuf outputColor;
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
void ffOptionParseColorNoClear(const char* value, FFstrbuf* buffer);
static inline void ffOptionParseColor(const char* value, FFstrbuf* buffer)
{
    ffStrbufClear(buffer);
    ffOptionParseColorNoClear(value, buffer);
}

static inline void ffOptionInitModuleArg(FFModuleArgs* args, const char* icon)
{
    ffStrbufInit(&args->key);
    ffStrbufInit(&args->keyColor);
    ffStrbufInitStatic(&args->keyIcon, icon);
    ffStrbufInit(&args->outputFormat);
    ffStrbufInit(&args->outputColor);
    args->keyWidth = 0;
}

static inline void ffOptionDestroyModuleArg(FFModuleArgs* args)
{
    ffStrbufDestroy(&args->key);
    ffStrbufDestroy(&args->keyColor);
    ffStrbufDestroy(&args->keyIcon);
    ffStrbufDestroy(&args->outputFormat);
    ffStrbufDestroy(&args->outputColor);
}
