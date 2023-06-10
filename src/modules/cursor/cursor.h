#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_CURSOR_MODULE_NAME "Cursor"

void ffPrintCursor(FFinstance* instance, FFCursorOptions* options);
void ffInitCursorOptions(FFCursorOptions* options);
bool ffParseCursorCommandOptions(FFCursorOptions* options, const char* key, const char* value);
void ffDestroyCursorOptions(FFCursorOptions* options);
void ffParseCursorJsonObject(FFinstance* instance, yyjson_val* module);
