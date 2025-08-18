#pragma once

#include "util/FFstrbuf.h"

struct yyjson_val;
struct yyjson_mut_doc;
struct yyjson_mut_val;

typedef struct FFModuleFormatArg
{
    const char* desc;
    const char* name;
} FFModuleFormatArg;

typedef struct FFModuleFormatArgList
{
    FFModuleFormatArg* args;
    uint32_t count;
} FFModuleFormatArgList;

#define FF_FORMAT_ARG_LIST(list) { .args = list, .count = sizeof(list) / sizeof(FFModuleFormatArg) }

// Must be the first field of FFModuleOptions
typedef struct FFModuleBaseInfo
{
    const char* name;
    const char* description;
    // A dirty polymorphic implementation in C.
    // This is UB, because `void*` is not compatible with `FF*Options*`.
    // However we can't do it better unless we move to C++, so that `option` becomes a `this` pointer
    // https://stackoverflow.com/questions/559581/casting-a-function-pointer-to-another-type

    void (*initOptions)(void* options);
    void (*destroyOptions)(void* options);
    void (*parseJsonObject)(void* options, struct yyjson_val *module);
    bool (*printModule)(void* options); // true on success
    bool (*generateJsonResult)(void* options, struct yyjson_mut_doc* doc, struct yyjson_mut_val* module); // true on success
    void (*generateJsonConfig)(void* options, struct yyjson_mut_doc* doc, struct yyjson_mut_val* obj);
    FFModuleFormatArgList formatArgs;
} FFModuleBaseInfo;

typedef enum __attribute__((__packed__)) FFModuleKeyType
{
    FF_MODULE_KEY_TYPE_NONE = 0,
    FF_MODULE_KEY_TYPE_STRING = 1 << 0,
    FF_MODULE_KEY_TYPE_ICON = 1 << 1,
    FF_MODULE_KEY_TYPE_BOTH = FF_MODULE_KEY_TYPE_STRING | FF_MODULE_KEY_TYPE_ICON,
    FF_MODULE_KEY_TYPE_FORCE_UNSIGNED = UINT8_MAX,
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

enum { FF_OPTION_MAX_SIZE = 1 << 8 }; // Maximum size of a single option value, used for static allocation
