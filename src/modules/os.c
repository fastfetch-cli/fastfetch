#include "fastfetch.h"

void ffPrintOS(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, "OS"))
        return;

    char name[256];
    ffParsePropFile("/etc/os-release", "NAME=\"%[^\"]+", name);
    if(name[0] == '\0' && ffStrbufIsEmpty(&instance->config.osFormat)) //With custom format string we pass an empty string as name
    {
        ffPrintError(instance, "OS", "\"NAME=\\\"%[^\\\"]+\" not found in \"/etc/os-release\"");
        return;
    }

    ffPrintLogoAndKey(instance, "OS");

    FF_STRBUF_CREATE(os);

    if(!ffStrbufIsEmpty(&instance->config.osFormat))
    {
        ffParseFormatString(&os, &instance->config.osFormat, 2,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, name},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.machine}
        );
    }
    else
    {
        ffStrbufSetF(&os, "%s %s", name, instance->state.utsname.machine);
    }

    ffPrintAndSaveCachedValue(instance, "OS", os.chars);
    ffStrbufDestroy(&os);
}