#include "fastfetch.h"

void ffPrintOS(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, "OS"))
        return;

    char name[256];
    ffParsePropFile("/etc/os-release", "NAME=\"%[^\"]+", name);
    if(name[0] == '\0')
    {
        ffPrintError(instance, "OS", "\"NAME=\\\"%[^\\\"]+\" not found in \"/etc/os-release\"");
        return;
    }

    char os[1024];
    sprintf(os, "%s %s", name, instance->state.utsname.machine);

    ffPrintAndSaveCachedValue(instance, "OS", os);
}