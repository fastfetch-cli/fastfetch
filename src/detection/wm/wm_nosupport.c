#include "wm.h"

const char* ffDetectWMPlugin([[maybe_unused]] FFstrbuf* pluginName) {
    return "Not supported on this platform";
}

const char* ffDetectWMVersion([[maybe_unused]] const FFstrbuf* wmName, [[maybe_unused]] FFstrbuf* result, [[maybe_unused]] FFWMOptions* options) {
    return "Not supported on this platform";
}
