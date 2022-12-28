#include "fastfetch.h"
#include "font.h"

void ffDetectFontImpl(const FFinstance* instance, FFFontResult* result)
{
    FF_UNUSED(instance);
    ffStrbufAppendS(&result->error, "Not supported on this platform");
}
