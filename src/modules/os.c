#include "fastfetch.h"

void ffPrintOS(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, "OS"))
        return;

    char name[256];
    ffParsePropFile("/etc/os-release", "NAME=\"%[^\"]+", name);
    if(name[0] == '\0' && instance->config.osFormat[0] == '\0') //With custom format string we pass an empty string as name
    {
        ffPrintError(instance, "OS", "\"NAME=\\\"%[^\\\"]+\" not found in \"/etc/os-release\"");
        return;
    }

    char os[1024];

    if(instance->config.osFormat[0] != '\0')
        sprintf(os, instance->config.osFormat, name, instance->state.utsname.machine);
    else if(instance->config.osShowArchitecture)
        sprintf(os, "%s %s", name, instance->state.utsname.machine);
    else
        strcpy(os, name);

    ffPrintAndSaveCachedValue(instance, "OS", os);
}