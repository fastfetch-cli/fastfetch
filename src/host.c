#include "fastfetch.h"

void ffPrintHost(FFstate* state)
{
    char name[256];
    ffReadFile("/sys/devices/virtual/dmi/id/product_name", name, 256);
    
    char version[256];
    ffReadFile("/sys/devices/virtual/dmi/id/product_version", version, 256);

    ffPrintLogoAndKey(state, "Host");
    printf("%s %s\n", name, version);
}