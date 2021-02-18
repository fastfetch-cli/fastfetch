#include "fastfetch.h"

void ffPrintLocale(FFstate* state)
{
    char locale[256];
    ffParsePropFile("/etc/locale.conf", "LANG=%[^\n]", locale);

    ffPrintLogoAndKey(state, "Locale");
    puts(locale);
}