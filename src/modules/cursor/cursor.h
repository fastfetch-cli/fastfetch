#pragma once

#include "fastfetch.h"

#define FF_CURSOR_MODULE_NAME "Cursor"

void ffPrintCursor(FFCursorOptions* options);
void ffInitCursorOptions(FFCursorOptions* options);
bool ffParseCursorCommandOptions(FFCursorOptions* options, const char* key, const char* value);
void ffDestroyCursorOptions(FFCursorOptions* options);
void ffParseCursorJsonObject(FFCursorOptions* options, yyjson_val* module);
