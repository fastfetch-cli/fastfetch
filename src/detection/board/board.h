#pragma once

#include "fastfetch.h"

typedef struct FFBoardResult
{
    FFstrbuf name;
    FFstrbuf vendor;
    FFstrbuf version;
} FFBoardResult;

const char* ffDetectBoard(FFBoardResult* board);
