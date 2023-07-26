#pragma once

#ifndef FF_INCLUDED_detection_terminalsize_terminalsize
#define FF_INCLUDED_detection_terminalsize_terminalsize

#include "fastfetch.h"

typedef struct FFTerminalSizeResult
{
    uint16_t rows;
    uint16_t columns;
    uint16_t width;
    uint16_t height;
} FFTerminalSizeResult;

bool ffDetectTerminalSize(FFTerminalSizeResult* result);

#endif
