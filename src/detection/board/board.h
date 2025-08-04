#pragma once

#include "fastfetch.h"
#include "modules/board/option.h"

typedef struct FFBoardResult
{
    FFstrbuf name;
    FFstrbuf vendor;
    FFstrbuf version;
    FFstrbuf serial;
} FFBoardResult;

const char* ffDetectBoard(FFBoardResult* board);
