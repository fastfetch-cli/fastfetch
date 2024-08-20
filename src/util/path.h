#pragma once

#include "fastfetch.h"

const char* ffFindExecutableInPath(const char* name, FFstrbuf* result);
bool ffIsAbsolutePath(const char* path);
