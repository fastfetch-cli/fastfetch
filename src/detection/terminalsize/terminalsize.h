#pragma once

#include "fastfetch.h"
#include "modules/terminalsize/option.h"

typedef struct FFTerminalSizeResult {
    uint16_t rows;
    uint16_t columns;
    uint16_t width;
    uint16_t height;
} FFTerminalSizeResult;

// `fastOnly` avoids terminal query/response sequences and is intended for startup layout decisions.
bool ffDetectTerminalSize(FFTerminalSizeResult* result, bool fastOnly);
