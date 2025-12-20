#pragma once

#include "option.h"

#define FF_SOUND_MODULE_NAME "Sound"

bool ffPrintSound(FFSoundOptions* options);
void ffInitSoundOptions(FFSoundOptions* options);
void ffDestroySoundOptions(FFSoundOptions* options);

extern FFModuleBaseInfo ffSoundModuleInfo;
