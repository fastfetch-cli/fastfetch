#pragma once

#include "option.h"

bool ffPrintDateTime(FFDateTimeOptions* options);
void ffInitDateTimeOptions(FFDateTimeOptions* options);
void ffDestroyDateTimeOptions(FFDateTimeOptions* options);

extern FFModuleBaseInfo ffDateTimeModuleInfo;
