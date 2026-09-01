#pragma once

#include "option.h"

bool ffPrintBoard(FFBoardOptions* options);
void ffInitBoardOptions(FFBoardOptions* options);
void ffDestroyBoardOptions(FFBoardOptions* options);

extern FFModuleBaseInfo ffBoardModuleInfo;
