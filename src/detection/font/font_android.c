#include "common/font.h"

const char* ffDetectFontImpl(FFinstance* instance, FFlist* result)
{
    FF_UNUSED(instance, result);
    return "Font detection is not supported on Android";
}
