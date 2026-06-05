#include "wm.h"

const char* ffDetectWMPlugin(FF_A_UNUSED FFstrbuf* pluginName) {
    return "Not supported on this platform";
}

const char* ffDetectWMVersion(FF_A_UNUSED const FFstrbuf* wmName, FF_A_UNUSED FFstrbuf* result, FF_A_UNUSED FFWMOptions* options) {
    return "Not supported on this platform";
}
