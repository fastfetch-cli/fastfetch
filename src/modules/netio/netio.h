#pragma once

#include "fastfetch.h"

#define FF_NETIO_MODULE_NAME "NetIO"

void ffPrepareNetIO(FFNetIOOptions* options);

void ffPrintNetIO(FFNetIOOptions* options);
void ffInitNetIOOptions(FFNetIOOptions* options);
void ffDestroyNetIOOptions(FFNetIOOptions* options);
