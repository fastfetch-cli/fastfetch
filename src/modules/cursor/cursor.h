#pragma once

#include "fastfetch.h"

#define FF_CURSOR_MODULE_NAME "Cursor"

void ffPrintCursor(FFCursorOptions* options);
void ffInitCursorOptions(FFCursorOptions* options);
void ffDestroyCursorOptions(FFCursorOptions* options);
