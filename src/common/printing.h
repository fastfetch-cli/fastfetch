#pragma once

#include "fastfetch.h"
#include "common/format.h"

typedef enum FFPrintType: uint8_t {
    FF_PRINT_TYPE_DEFAULT = 0,
    FF_PRINT_TYPE_NO_CUSTOM_KEY = 1 << 0, // key has been formatted outside
    FF_PRINT_TYPE_NO_CUSTOM_KEY_COLOR = 1 << 1,
    FF_PRINT_TYPE_NO_CUSTOM_KEY_WIDTH = 1 << 2,
    FF_PRINT_TYPE_NO_CUSTOM_OUTPUT_FORMAT = 1 << 3, // reserved
} FFPrintType;

// Both `moduleName` and `moduleArgs` are optional and explicitly handled: a null `moduleName` is what
// `--set-keyless` relies on, and a null `moduleArgs` (used by ffPrintError callers) skips the custom
// key entirely. So neither takes `nonnull`.
void ffPrintLogoAndKey(const char* moduleName, uint32_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType);
// Same for `moduleArgs`; `numArgs` / `arguments` are only read when `moduleArgs` is non-null, so
// `arguments` is not `nonnull(6)` either. At least one caller discards the result, so not `nodiscard`.
bool ffPrintFormat(const char* moduleName, uint32_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType, uint32_t numArgs, const FFformatarg* arguments);
#define FF_PRINT_FORMAT_CHECKED(moduleName, moduleIndex, moduleArgs, printType, arguments) \
    ffPrintFormat((moduleName), (moduleIndex), (moduleArgs), (printType), (sizeof(arguments) / sizeof(*arguments)), (arguments));
// `moduleName` / `moduleArgs` are forwarded to ffPrintLogoAndKey, which accepts null for both;
// only `message` is dereferenced here (by `vprintf`).
[[gnu::format(printf, 5, 6), gnu::nonnull(5)]] void ffPrintError(const char* moduleName, uint32_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType, const char* message, ...);
[[gnu::nonnull(1)]] void ffPrintColor(const FFstrbuf* colorValue);
void ffPrintCharTimes(char c, uint32_t times);
