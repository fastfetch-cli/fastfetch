#include "common/font.h"
#include "common/io/io.h"
#include "font.h"

#import <AppKit/NSFont.h>

void ffDetectFontImpl(const FFinstance* instance, FFFontResult* result)
{
    FF_UNUSED(instance);
    ffStrbufAppendS(&result->fonts[0], [NSFont systemFontOfSize:12].familyName.UTF8String);
    ffStrbufAppendS(&result->fonts[1], [NSFont userFontOfSize:12].familyName.UTF8String);
    ffStrbufAppendS(&result->fonts[2], [NSFont monospacedSystemFontOfSize:12 weight:400].familyName.UTF8String);
    ffStrbufAppendS(&result->fonts[3], [NSFont userFixedPitchFontOfSize:12].familyName.UTF8String);
}
