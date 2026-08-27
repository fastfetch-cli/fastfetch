#pragma once

#include "option.h"

bool ffPrintSound(FFSoundOptions* options);
void ffInitSoundOptions(FFSoundOptions* options);
void ffDestroySoundOptions(FFSoundOptions* options);

extern FFModuleBaseInfo ffSoundModuleInfo;
