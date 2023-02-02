#pragma once

#ifndef FF_INCLUDED_util_stringUtils_h
#define FF_INCLUDED_util_stringUtils_h

#include <stdbool.h>
#include <stdint.h>

bool ffStrSet(const char* str);
bool ffStrHasNChars(const char* str, char c, uint32_t n);

#endif
