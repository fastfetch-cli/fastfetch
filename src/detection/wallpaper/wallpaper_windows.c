#include "wallpaper.h"
#include "common/windows/registry.h"

const char* ffDetectWallpaper(FFstrbuf* result)
{
    FF_AUTO_CLOSE_FD HANDLE hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Control Panel\\Desktop", &hKey, NULL))
        return "ffRegOpenKeyForRead(Control Panel\\Desktop) failed";

    if(!ffRegReadStrbuf(hKey, L"WallPaper", result, NULL))
        return "ffRegReadStrbuf(WallPaper) failed";

    return NULL;
}
