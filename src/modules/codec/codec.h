#pragma once

#include "option.h"

bool ffPrintCodec(FFCodecOptions* options);
void ffInitCodecOptions(FFCodecOptions* options);
void ffDestroyCodecOptions(FFCodecOptions* options);

extern FFModuleBaseInfo ffCodecModuleInfo;
