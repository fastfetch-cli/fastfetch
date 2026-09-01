#pragma once

#include "option.h"

bool ffPrintTPM(FFTPMOptions* options);
void ffInitTPMOptions(FFTPMOptions* options);
void ffDestroyTPMOptions(FFTPMOptions* options);

extern FFModuleBaseInfo ffTPMModuleInfo;
