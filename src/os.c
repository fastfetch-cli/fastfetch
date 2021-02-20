#include "fastfetch.h"

void ffPrintOS(FFstate* state)
{
    char name[256];
    ffParsePropFile("/etc/os-release", "NAME=\"%[^\"]+", name);
    if(name[0] == '\0')
    {
        ffPrintError(state, "OS", "\"NAME=\\\"%[^\\\"]+\" not found in \"/etc/os-release\"");
        return;
    }

    ffPrintLogoAndKey(state, "OS");
    printf("%s %s\n", name, state->utsname.machine);
}