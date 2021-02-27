#include "fastfetch.h"

void ffPrintLocale(FFstate* state)
{
    if(ffPrintCachedValue(state, "Locale"))
        return;

    char locale[256];
    ffParsePropFile("/etc/locale.conf", "LANG=%[^\n]", locale);
    if(locale[0] == '\0')
    {
        ffPrintError(state, "Locale", "\"LANG=%[^\\n]\" not found in \"/etc/locale.conf\"");
        return;
    }

    ffPrintAndSaveCachedValue(state, "Locale", locale);
}