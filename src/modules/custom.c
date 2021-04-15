#include "fastfetch.h"

void ffPrintCustom(FFinstance* instance, const char* key, const char* value)
{
    ffPrintLogoAndKey(instance, key, 0, NULL);
    puts(value);
}
