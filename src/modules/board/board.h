#pragma once

#include "option.h"

#define FF_BOARD_MODULE_NAME "Board"

void ffPrintBoard(FFBoardOptions* options);
void ffInitBoardOptions(FFBoardOptions* options);
void ffDestroyBoardOptions(FFBoardOptions* options);

extern FFModuleBaseInfo ffBoardModuleInfo;
