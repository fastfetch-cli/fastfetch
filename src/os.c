#include "fastfetch.h"

void ffPrintOS(FFstate* state)
{
    if(ffPrintCachedValue(state, "OS"))
        return;

    char name[256];
    ffParsePropFile("/etc/os-release", "NAME=\"%[^\"]+", name);
    if(name[0] == '\0')
    {
        ffPrintError(state, "OS", "\"NAME=\\\"%[^\\\"]+\" not found in \"/etc/os-release\"");
        return;
    }

    char os[1024];
    sprintf(os, "%s %s", name, state->utsname.machine);

    ffSaveCachedValue(state, "OS", os);

    ffPrintLogoAndKey(state, "OS");
    puts(os);
}