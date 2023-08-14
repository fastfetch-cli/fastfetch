#pragma once

#ifndef FF_INCLUDED_detection_board_board
#define FF_INCLUDED_detection_board_board

#include "fastfetch.h"

typedef struct FFBoardResult
{
    FFstrbuf name;
    FFstrbuf vendor;
    FFstrbuf version;
} FFBoardResult;

const char* ffDetectBoard(FFBoardResult* result);

#endif
