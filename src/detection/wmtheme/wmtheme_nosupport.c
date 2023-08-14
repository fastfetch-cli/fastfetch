#include "fastfetch.h"
#include "wmtheme.h"

bool ffDetectWmTheme(FFstrbuf* themeOrError)
{
    ffStrbufAppendS(themeOrError, "Not supported on this platform");
    return false;
}
