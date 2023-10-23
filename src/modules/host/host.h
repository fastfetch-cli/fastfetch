#pragma once

#include "fastfetch.h"

#define FF_HOST_MODULE_NAME "Host"

void ffPrintHost(FFHostOptions* options);
void ffInitHostOptions(FFHostOptions* options);
void ffDestroyHostOptions(FFHostOptions* options);
