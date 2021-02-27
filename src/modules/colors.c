#include "fastfetch.h"

void ffPrintColors(FFinstance* instance)
{
    ffPrintLogoLine(instance);

    for(uint8_t i = 0; i < 8; i++)
        printf("\033[4%dm   ", i);

    puts("\033[0m");

    ffPrintLogoLine(instance);

    for(uint8_t i = 8; i < 16; i++)
        printf("\033[48;5;%dm   ", i);

    puts("\033[0m");
}