#include "wallpaper.h"
#include "common/debug.h"
#include "common/windows/registry.h"

const char* ffDetectWallpaper(FFstrbuf* result) {
    FF_AUTO_CLOSE_FD HANDLE hKey = nullptr;
    if (!ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Control Panel\\Desktop", &hKey, nullptr)) {
        FF_DEBUG("ffRegOpenKeyForRead(Control Panel\\Desktop) failed");
        return "ffRegOpenKeyForRead(Control Panel\\Desktop) failed";
    }

    if (!ffRegReadStrbuf(hKey, L"WallPaper", result, nullptr)) {
        FF_DEBUG("ffRegReadStrbuf(WallPaper) failed");
        return "ffRegReadStrbuf(WallPaper) failed";
    }

    return nullptr;
}
