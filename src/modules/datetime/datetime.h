#pragma once

#include "option.h"

#define FF_DATETIME_MODULE_NAME "DateTime"

bool ffPrintDateTime(FFDateTimeOptions* options);
void ffInitDateTimeOptions(FFDateTimeOptions* options);
void ffDestroyDateTimeOptions(FFDateTimeOptions* options);

extern FFModuleBaseInfo ffDateTimeModuleInfo;
