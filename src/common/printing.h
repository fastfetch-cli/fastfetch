#pragma once

#ifndef FF_INCLUDED_common_printing
#define FF_INCLUDED_common_printing

#include "fastfetch.h"
#include "common/format.h"

void ffPrintLogoAndKey(const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* customKeyColor);
void ffPrintFormatString(const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* customKeyColor, const FFstrbuf* format, uint32_t numArgs, const FFformatarg* arguments);
void ffPrintFormat(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, uint32_t numArgs, const FFformatarg* arguments);
FF_C_PRINTF(5, 6) void ffPrintErrorString(const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* customKeyColor, const char* message, ...);
FF_C_PRINTF(4, 5) void ffPrintError(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, const char* message, ...);
void ffPrintColor(const FFstrbuf* colorValue);
void ffPrintCharTimes(char c, uint32_t times);

#endif
