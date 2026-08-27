#include "fastfetch.h"
#include "font.h"

const char* ffDetectFontImpl([[maybe_unused]] FFFontResult* result) {
    FF_UNUSED(result);
    return "Not supported on this platform";
}
