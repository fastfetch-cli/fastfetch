#include "cursor.h"

#include "common/io.h"
#include "common/windows/registry.h"

void ffDetectCursor(FFCursorResult* result) {
    FF_AUTO_CLOSE_FD HANDLE hKey = NULL;
    if (ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Control Panel\\Cursors", &hKey, &result->error)) {
        if (ffRegReadStrbuf(hKey, NULL, &result->theme, &result->error)) {
            uint32_t cursorBaseSize;
            if (ffRegReadUint(hKey, L"CursorBaseSize", &cursorBaseSize, NULL)) {
                // Not available on Windows 8.1
                ffStrbufAppendUInt(&result->size, cursorBaseSize);
            };
        }
    }
}
