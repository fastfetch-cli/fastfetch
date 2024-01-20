#pragma once

#include "fastfetch.h"

enum
{
    FF_PERCENTAGE_TYPE_NUM_BIT = 1 << 0,
    FF_PERCENTAGE_TYPE_BAR_BIT = 1 << 1,
    FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT = 1 << 2,
    FF_PERCENTAGE_TYPE_NUM_COLOR_BIT = 1 << 3,
};

// if (green <= yellow)
// [0, green]: print green
// (green, yellow]: print yellow
// (yellow, 100]: print red
//
// if (green > yellow)
// [green, 100]: print green
// [yellow, green): print yellow
// [0, yellow): print red

void ffPercentAppendBar(FFstrbuf* buffer, double percent, uint8_t green, uint8_t yellow);
void ffPercentAppendNum(FFstrbuf* buffer, double percent, uint8_t green, uint8_t yellow, bool parentheses);
