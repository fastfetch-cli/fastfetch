#pragma once

#include "fastfetch.h"
#include "modules/title/option.h"

#define FF_SOUND_MODULE_NAME "Sound"

void ffPrintSound(FFinstance* instance, FFSoundOptions* options);
void ffInitSoundOptions(FFSoundOptions* options);
bool ffParseSoundCommandOptions(FFSoundOptions* options, const char* key, const char* value);
void ffDestroySoundOptions(FFSoundOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseSoundJsonObject(FFinstance* instance, json_object* module);
#endif
