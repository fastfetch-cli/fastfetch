#pragma once

#include "fastfetch.h"

#define FF_BIOS_MODULE_NAME "Bios"

void ffPrintBios(FFBiosOptions* options);
void ffInitBiosOptions(FFBiosOptions* options);
void ffDestroyBiosOptions(FFBiosOptions* options);
