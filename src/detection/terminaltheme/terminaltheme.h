#pragma once

#include "fastfetch.h"

typedef struct FFTerminalThemeColor
{
    uint16_t r;
    uint16_t g;
    uint16_t b;
    bool dark;
} FFTerminalThemeColor;

typedef struct FFTerminalThemeResult
{
    FFTerminalThemeColor fg;
    FFTerminalThemeColor bg;
} FFTerminalThemeResult;

bool ffDetectTerminalTheme(FFTerminalThemeResult* result, bool forceEnv);
