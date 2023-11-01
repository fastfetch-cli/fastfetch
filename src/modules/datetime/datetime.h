#pragma once

#include "fastfetch.h"

#define FF_DATETIME_MODULE_NAME "DateTime"

void ffPrintDateTime(FFDateTimeOptions* options);
void ffInitDateTimeOptions(FFDateTimeOptions* options);
void ffDestroyDateTimeOptions(FFDateTimeOptions* options);
