#pragma once

#include "fastfetch.h"
#include "common/format.h"

typedef enum FFPrintType {
    FF_PRINT_TYPE_DEFAULT = 0,
    FF_PRINT_TYPE_NO_CUSTOM_KEY = 1 << 0, // key has been formatted outside
    FF_PRINT_TYPE_NO_CUSTOM_KEY_COLOR = 1 << 1,
    FF_PRINT_TYPE_NO_CUSTOM_KEY_WIDTH = 1 << 2,
    FF_PRINT_TYPE_NO_CUSTOM_OUTPUT_FORMAT = 1 << 3, // reserved
} FFPrintType;

void ffPrintLogoAndKey(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType);
void ffPrintFormat(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType, uint32_t numArgs, const FFformatarg* arguments);
#define FF_PRINT_FORMAT_CHECKED(moduleName, moduleIndex, moduleArgs, printType, numArgs, arguments) do {\
    static_assert(sizeof(arguments) / sizeof(*arguments) == (numArgs), "Invalid number of format arguments");\
    ffPrintFormat((moduleName), (moduleIndex), (moduleArgs), (printType), (numArgs), (arguments));\
} while (0)
FF_C_PRINTF(5, 6) void ffPrintError(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType, const char* message, ...);
void ffPrintColor(const FFstrbuf* colorValue);
void ffPrintCharTimes(char c, uint32_t times);

void ffPrintModuleFormatHelp(const char* name, const char* def, uint32_t numArgs, const char* args[]);
#define FF_PRINT_MODULE_FORMAT_HELP_CHECKED(moduleName, def, numArgs, args) do {\
    static_assert(sizeof(args) / sizeof(*args) == (numArgs), "Invalid number of format arguments");\
    ffPrintModuleFormatHelp((moduleName), (def), (numArgs), (args));\
} while (0)
