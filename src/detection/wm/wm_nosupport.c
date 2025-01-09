#include "wm.h"

const char* ffDetectWMPlugin(FF_MAYBE_UNUSED FFstrbuf* pluginName)
{
    return "Not supported on this platform";
}

const char* ffDetectWMVersion(FF_MAYBE_UNUSED const FFstrbuf* wmName, FF_MAYBE_UNUSED FFstrbuf* result, FF_MAYBE_UNUSED FFWMOptions* options)
{
    return "Not supported on this platform";
}
