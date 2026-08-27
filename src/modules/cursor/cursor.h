#pragma once

#include "option.h"

bool ffPrintCursor(FFCursorOptions* options);
void ffInitCursorOptions(FFCursorOptions* options);
void ffDestroyCursorOptions(FFCursorOptions* options);

extern FFModuleBaseInfo ffCursorModuleInfo;
