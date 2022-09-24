#include "fastfetch.h"
#include "detection/displayserver/displayserver.h"
#include "wmtheme.h"

bool ffDetectWmTheme(FFinstance* instance, FFstrbuf* themeOrError)
{
    FF_UNUSED(instance);
    ffStrbufAppendS(themeOrError, "WM theme detection is not supported on Android");
    return false;
}
