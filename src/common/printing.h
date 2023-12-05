#pragma once

#include "fastfetch.h"
#include "common/format.h"

typedef enum FFPrintType {
    FF_PRINT_TYPE_DEFAULT = 0,
    FF_PRINT_TYPE_NO_CUSTOM_KEY = 1 << 0,
    FF_PRINT_TYPE_NO_CUSTOM_KEY_COLOR = 1 << 1,
    FF_PRINT_TYPE_NO_CUSTOM_KEY_WIDTH = 1 << 2,
    FF_PRINT_TYPE_NO_CUSTOM_OUTPUT_FORMAT = 1 << 3, // reserved
} FFPrintType;

void ffPrintLogoAndKey(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType);
void ffPrintFormatString(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType, uint32_t numArgs, const FFformatarg* arguments);
static inline void ffPrintFormat(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, uint32_t numArgs, const FFformatarg* arguments)
{
    ffPrintFormatString(moduleName, moduleIndex, moduleArgs, FF_PRINT_TYPE_DEFAULT, numArgs, arguments);
}
FF_C_PRINTF(5, 6) void ffPrintErrorString(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType, const char* message, ...);
FF_C_PRINTF(4, 5) void ffPrintError(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, const char* message, ...);
void ffPrintColor(const FFstrbuf* colorValue);
void ffPrintCharTimes(char c, uint32_t times);

void ffPrintModuleFormatHelp(const char* name, const char* def, uint32_t numArgs, const char* args[]);
