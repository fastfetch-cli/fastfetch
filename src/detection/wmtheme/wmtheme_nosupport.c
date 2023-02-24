#include "fastfetch.h"
#include "wmtheme.h"

bool ffDetectWmTheme(FFinstance* instance, FFstrbuf* themeOrError)
{
    FF_UNUSED(instance);
    ffStrbufAppendS(themeOrError, "Not supported on this platform");
    return false;
}
