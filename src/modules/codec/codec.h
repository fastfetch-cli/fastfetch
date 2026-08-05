#pragma once

#include "option.h"

#define FF_CODEC_MODULE_NAME "Codec"

bool ffPrintCodec(FFCodecOptions* options);
void ffInitCodecOptions(FFCodecOptions* options);
void ffDestroyCodecOptions(FFCodecOptions* options);

extern FFModuleBaseInfo ffCodecModuleInfo;
