#include "fastfetch.h"

void ffPrintKernel(FFinstance* instance)
{
    ffPrintLogoAndKey(instance, "Kernel");
    puts(instance->state.utsname.release);
}