#include "fastfetch.h"

void ffPrintLocale(FFstate* state)
{
    char locale[256];
    ffParsePropFile("/etc/locale.conf", "LANG=%[^\n]", locale);
    if(locale[0] == '\0')
    {
        ffPrintError(state, "Locale", "\"LANG=%[^\\n]\" not found in \"/etc/locale.conf\"");
        return;
    }

    ffPrintLogoAndKey(state, "Locale");
    puts(locale);
}