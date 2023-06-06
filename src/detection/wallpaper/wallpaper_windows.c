#include "wallpaper.h"
#include "util/windows/registry.h"

const char* ffDetectWallpaper(FF_MAYBE_UNUSED const FFinstance* instance, FFstrbuf* result)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Control Panel\\Desktop", &hKey, NULL))
        return "ffRegOpenKeyForRead(Control Panel\\Desktop) failed";

    if(!ffRegReadStrbuf(hKey, L"WallPaper", result, NULL))
        return "ffRegReadStrbuf(WallPaper) failed";

    return NULL;
}
