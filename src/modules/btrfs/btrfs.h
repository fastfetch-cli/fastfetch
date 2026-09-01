#pragma once

#include "option.h"

bool ffPrintBtrfs(FFBtrfsOptions* options);
void ffInitBtrfsOptions(FFBtrfsOptions* options);
void ffDestroyBtrfsOptions(FFBtrfsOptions* options);

extern FFModuleBaseInfo ffBtrfsModuleInfo;
