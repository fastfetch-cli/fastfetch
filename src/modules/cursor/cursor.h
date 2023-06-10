#pragma once

#include "fastfetch.h"

#define FF_CURSOR_MODULE_NAME "Cursor"

void ffPrintCursor(FFinstance* instance, FFCursorOptions* options);
void ffInitCursorOptions(FFCursorOptions* options);
bool ffParseCursorCommandOptions(FFCursorOptions* options, const char* key, const char* value);
void ffDestroyCursorOptions(FFCursorOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseCursorJsonObject(FFinstance* instance, json_object* module);
#endif
