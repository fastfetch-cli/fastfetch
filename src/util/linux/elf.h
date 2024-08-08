#pragma once

#include "fastfetch.h"

const char* ffElfExtractStrings(const char* elfFile, bool (*cb)(const char* str, uint32_t len, void* userdata), void* userdata);
