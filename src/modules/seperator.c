#include "fastfetch.h"

void ffPrintSeperator(FFstate* state)
{
    ffPrintLogoLine(state);

    for(uint8_t i = 0; i < state->titleLength; i++)
        putchar('-');
    putchar('\n');
}