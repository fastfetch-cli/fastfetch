#pragma once

#include "option.h"

#define FF_TPM_MODULE_NAME "TPM"

void ffPrintTPM(FFTPMOptions* options);
void ffInitTPMOptions(FFTPMOptions* options);
void ffDestroyTPMOptions(FFTPMOptions* options);

extern FFModuleBaseInfo ffTPMModuleInfo;
