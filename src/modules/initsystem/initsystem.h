#pragma once

#include "fastfetch.h"

#define FF_INITSYSTEM_MODULE_NAME "InitSystem"

void ffPrintInitSystem(FFInitSystemOptions* options);
void ffInitInitSystemOptions(FFInitSystemOptions* options);
void ffDestroyInitSystemOptions(FFInitSystemOptions* options);
