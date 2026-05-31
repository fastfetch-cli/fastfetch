#pragma once

#include "option.h"

#define FF_DECODER_MODULE_NAME "Decoder"

bool ffPrintDecoder(FFDecoderOptions* options);
void ffInitDecoderOptions(FFDecoderOptions* options);
void ffDestroyDecoderOptions(FFDecoderOptions* options);

extern FFModuleBaseInfo ffDecoderModuleInfo;
