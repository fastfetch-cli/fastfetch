#pragma once

#ifndef FF_INCLUDED_common_printing
#define FF_INCLUDED_common_printing

#include "fastfetch.h"
#include "common/format.h"

void ffPrintLogoAndKey(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat);
void ffPrintFormatString(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* format, uint32_t numArgs, const FFformatarg* arguments);
void ffPrintFormat(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, uint32_t numArgs, const FFformatarg* arguments);
FF_C_PRINTF(6, 7) void ffPrintErrorString(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* customErrorFormat, const char* message, ...);
FF_C_PRINTF(5, 6) void ffPrintError(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, const char* message, ...);
void ffPrintColor(const FFstrbuf* colorValue);
void ffPrintCharTimes(char c, uint32_t times);
void ffPrintUserString(const char* str);

#endif
