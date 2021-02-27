#include "fastfetch.h"

void ffPrintCustom(FFinstance* instance, const char* key, const char* value)
{
    ffPrintLogoAndKey(instance, key);
    puts(value);
}