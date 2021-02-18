#include "fastfetch.h"

void ffPrintKernel(FFstate* state)
{
    ffPrintLogoAndKey(state, "Kernel");
    printf("%s\n", state->utsname.release);
}