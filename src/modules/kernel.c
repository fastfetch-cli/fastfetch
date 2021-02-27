#include "fastfetch.h"

void ffPrintKernel(FFinstance* instance)
{
    ffPrintLogoAndKey(instance, "Kernel");
    printf("%s\n", instance->state.utsname.release);
}