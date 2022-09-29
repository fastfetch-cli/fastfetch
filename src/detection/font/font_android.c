#include "fastfetch.h"

void ffDetectFontImpl(const FFinstance* instance, FFFontResult* result)
{
    FF_UNUSED(instance);
    ffStrbufAppendS(&result->error, "Not implemented");
}
