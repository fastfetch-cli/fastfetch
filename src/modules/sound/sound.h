#pragma once

#include "fastfetch.h"

#define FF_SOUND_MODULE_NAME "Sound"

void ffPrintSound(FFSoundOptions* options);
void ffInitSoundOptions(FFSoundOptions* options);
bool ffParseSoundCommandOptions(FFSoundOptions* options, const char* key, const char* value);
void ffDestroySoundOptions(FFSoundOptions* options);
void ffParseSoundJsonObject(yyjson_val* module);
