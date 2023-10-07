#pragma once

#include "fastfetch.h"

#define FF_CURSOR_MODULE_NAME "Cursor"

void ffPrintCursor(FFCursorOptions* options);
void ffInitCursorOptions(FFCursorOptions* options);
bool ffParseCursorCommandOptions(FFCursorOptions* options, const char* key, const char* value);
void ffDestroyCursorOptions(FFCursorOptions* options);
void ffParseCursorJsonObject(FFCursorOptions* options, yyjson_val* module);
void ffGenerateCursorJson(FFCursorOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintCursorHelpFormat(void);
