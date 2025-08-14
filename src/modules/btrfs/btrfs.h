#pragma once

#include "option.h"

#define FF_BTRFS_MODULE_NAME "Btrfs"

void ffPrintBtrfs(FFBtrfsOptions* options);
void ffInitBtrfsOptions(FFBtrfsOptions* options);
void ffDestroyBtrfsOptions(FFBtrfsOptions* options);

extern FFModuleBaseInfo ffBtrfsModuleInfo;
