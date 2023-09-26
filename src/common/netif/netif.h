#pragma once

#include "fastfetch.h"

#ifndef _WIN32
bool ffNetifGetDefaultRoute(FFstrbuf* iface);
#else
bool ffNetifGetDefaultRoute(uint32_t* ifIndex);
#endif
