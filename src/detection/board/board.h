#pragma once

#ifndef FF_INCLUDED_detection_board_board
#define FF_INCLUDED_detection_board_board

#include "fastfetch.h"

typedef struct FFBoardResult
{
    FFstrbuf boardName;
    FFstrbuf boardVendor;
    FFstrbuf boardVersion;
} FFBoardResult;

const char* ffDetectBoard(FFBoardResult* result);

#endif
