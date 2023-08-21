#pragma once

#ifndef FF_INCLUDED_common_commandoption
#define FF_INCLUDED_common_commandoption

#include "fastfetch.h"

bool ffParseModuleCommand(const char* type);
bool ffParseModuleOptions(const char* key, const char* value);

#endif
