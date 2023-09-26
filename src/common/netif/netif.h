#pragma once

#include "fastfetch.h"

#ifndef _WIN32
const char* ffNetifGetDefaultRoute();
#else
uint32_t ffNetifGetDefaultRoute();
#endif
