#include "fastfetch.h"

void ffPrintLocale(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, "Locale"))
        return;

    char locale[256];
    ffParsePropFile("/etc/locale.conf", "LANG=%[^\n]", locale);
    if(locale[0] == '\0')
    {
        ffPrintError(instance, "Locale", "\"LANG=%[^\\n]\" not found in \"/etc/locale.conf\"");
        return;
    }

    ffPrintAndSaveCachedValue(instance, "Locale", locale);
}