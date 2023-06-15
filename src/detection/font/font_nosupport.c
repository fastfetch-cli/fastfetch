#include "fastfetch.h"
#include "font.h"

const char* ffDetectFontImpl(const FFinstance* instance, FFFontResult* result)
{
    FF_UNUSED(instance, result);
    return "Not supported on this platform";
}
