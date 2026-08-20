#pragma once

#include "common/FFstrbuf.h"

struct yyjson_val;
struct yyjson_mut_doc;
struct yyjson_mut_val;

typedef struct FFModuleFormatArg {
    const char* desc;
    const char* name;
} FFModuleFormatArg;

typedef struct FFModuleFormatArgList {
    FFModuleFormatArg* args;
    uint32_t count;
} FFModuleFormatArgList;

#define FF_FORMAT_ARG_LIST(list) { .args = list, .count = sizeof(list) / sizeof(FFModuleFormatArg) }

typedef struct FFModuleDisplayName {
    const char* en; // English

    const char* ar; // Arabic
    const char* de; // German
    const char* es; // Spanish
    const char* fr; // French
    const char* gl; // Galician
    const char* he; // Hebrew
    const char* it; // Italian
    const char* ja; // Japanese
    const char* ko; // Korean
    const char* pl; // Polish
    const char* pt; // (Brazilian) Portuguese
    const char* ru; // Russian
    const char* zh_CN; // Simplified Chinese
    const char* zh_TW; // Traditional Chinese
} FFModuleDisplayName;

// Must be the first field of FFModuleOptions
typedef struct FFModuleBaseInfo {
    const char* name;
    const char* description;
    FFModuleDisplayName displayName;
    // A dirty polymorphic implementation in C.
    // This is UB, because `void*` is not compatible with `FF*Options*`.
    // However we can't do it better unless we move to C++, so that `option` becomes a `this` pointer
    // https://stackoverflow.com/questions/559581/casting-a-function-pointer-to-another-type

    void (*initOptions)(void* options);
    void (*destroyOptions)(void* options);
    void (*parseJsonObject)(void* options, struct yyjson_val* module);
    bool (*printModule)(void* options);                                                                   // true on success
    bool (*generateJsonResult)(void* options, struct yyjson_mut_doc* doc, struct yyjson_mut_val* module); // true on success
    void (*generateJsonConfig)(void* options, struct yyjson_mut_doc* doc, struct yyjson_mut_val* obj);
    FFModuleFormatArgList formatArgs;
    const uint8_t defaultOrder;
} FFModuleBaseInfo;

typedef enum FFModuleKeyType: uint8_t {
    FF_MODULE_KEY_TYPE_NONE = 0,
    FF_MODULE_KEY_TYPE_STRING = 1 << 0,
    FF_MODULE_KEY_TYPE_ICON = 1 << 1,
    FF_MODULE_KEY_TYPE_SPACE_SHIFT = 4,
    FF_MODULE_KEY_TYPE_BOTH_0 = FF_MODULE_KEY_TYPE_STRING | FF_MODULE_KEY_TYPE_ICON,
    FF_MODULE_KEY_TYPE_BOTH_1 = FF_MODULE_KEY_TYPE_BOTH_0 | (1 << FF_MODULE_KEY_TYPE_SPACE_SHIFT),
    FF_MODULE_KEY_TYPE_BOTH = FF_MODULE_KEY_TYPE_BOTH_1, // alias
    FF_MODULE_KEY_TYPE_BOTH_2 = FF_MODULE_KEY_TYPE_BOTH_0 | (2 << FF_MODULE_KEY_TYPE_SPACE_SHIFT),
    FF_MODULE_KEY_TYPE_BOTH_3 = FF_MODULE_KEY_TYPE_BOTH_0 | (3 << FF_MODULE_KEY_TYPE_SPACE_SHIFT),
    FF_MODULE_KEY_TYPE_BOTH_4 = FF_MODULE_KEY_TYPE_BOTH_0 | (4 << FF_MODULE_KEY_TYPE_SPACE_SHIFT),
} FFModuleKeyType;

typedef struct FFModuleArgs {
    FFstrbuf key;
    FFstrbuf keyColor;
    FFstrbuf keyIcon;
    FFstrbuf outputFormat;
    FFstrbuf outputColor;
    uint32_t keyWidth;
} FFModuleArgs;

typedef struct FFKeyValuePair {
    const char* key;
    int value;
} FFKeyValuePair;

const char* ffOptionTestPrefix(const char* argumentKey, const char* moduleName);
void ffOptionParseString(const char* argumentKey, const char* value, FFstrbuf* buffer);
[[nodiscard]] uint32_t ffOptionParseUInt32(const char* argumentKey, const char* value);
[[nodiscard]] int32_t ffOptionParseInt32(const char* argumentKey, const char* value);
[[nodiscard]] int ffOptionParseEnum(const char* argumentKey, const char* requestedKey, FFKeyValuePair pairs[]);
[[nodiscard]] bool ffOptionParseBoolean(const char* str);
void ffOptionParseColorNoClear(const char* value, FFstrbuf* buffer);
static inline void ffOptionParseColor(const char* value, FFstrbuf* buffer) {
    ffStrbufClear(buffer);
    ffOptionParseColorNoClear(value, buffer);
}

static inline void ffOptionInitModuleArg(FFModuleArgs* args, const char* icon) {
    ffStrbufInit(&args->key);
    ffStrbufInit(&args->keyColor);
    ffStrbufInitStatic(&args->keyIcon, icon);
    ffStrbufInit(&args->outputFormat);
    ffStrbufInit(&args->outputColor);
    args->keyWidth = 0;
}

static inline void ffOptionDestroyModuleArg(FFModuleArgs* args) {
    ffStrbufDestroy(&args->key);
    ffStrbufDestroy(&args->keyColor);
    ffStrbufDestroy(&args->keyIcon);
    ffStrbufDestroy(&args->outputFormat);
    ffStrbufDestroy(&args->outputColor);
}

enum { FF_OPTION_MAX_SIZE = 1 << 8 }; // Maximum size of a single option value, used for static allocation

#define FF_MODULE_GET_DISPLAY_NAME(moduleName) (*(const char**) ((uint8_t*) &ff ## moduleName ## ModuleInfo.displayName + instance.config.display.keyLanguage))
