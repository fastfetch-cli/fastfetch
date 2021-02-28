#include "fastfetch.h"

void ffPrintKernel(FFinstance* instance)
{
    #ifdef FASTFETCH_BUILD_FLASHFETCH
    if(ffPrintCustomValue(instance, "Kernel"))
        return;
    #endif // FASTFETCH_BUILD_FLASHFETCH

    ffPrintLogoAndKey(instance, "Kernel");
    puts(instance->state.utsname.release);
}