#pragma once

#include "fastfetch.h"

typedef struct FFTerminalSizeResult
{
    uint16_t rows;
    uint16_t columns;
    uint16_t width;
    uint16_t height;
} FFTerminalSizeResult;

bool ffDetectTerminalSize(FFTerminalSizeResult* result);
