#pragma once

#include "fastfetch.h"

#ifndef _WIN32
const char* ffNetifGetDefaultRouteIfName();
#endif

uint32_t ffNetifGetDefaultRouteIfIndex();
