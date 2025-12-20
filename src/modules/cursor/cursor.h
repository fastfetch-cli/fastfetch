#pragma once

#include "option.h"

#define FF_CURSOR_MODULE_NAME "Cursor"

bool ffPrintCursor(FFCursorOptions* options);
void ffInitCursorOptions(FFCursorOptions* options);
void ffDestroyCursorOptions(FFCursorOptions* options);

extern FFModuleBaseInfo ffCursorModuleInfo;
