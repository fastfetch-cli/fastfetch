#include "fastfetch.h"
#include "common/printing.h"

void ffPrintCustom(FFinstance* instance, const char* key, const char* value)
{
    ffPrintLogoAndKey(instance, key, 0, NULL);
    ffPrintUserString(value);
    puts(FASTFETCH_TEXT_MODIFIER_RESET);
}
