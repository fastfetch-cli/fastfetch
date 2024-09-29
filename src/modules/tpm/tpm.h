#pragma once

#include "fastfetch.h"

#define FF_TPM_MODULE_NAME "TPM"

void ffPrintTPM(FFTPMOptions* options);
void ffInitTPMOptions(FFTPMOptions* options);
void ffDestroyTPMOptions(FFTPMOptions* options);
