#pragma once

#include "fastfetch.h"

const char* ffBinaryExtractStrings(const char* file, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata, uint32_t minLength);
