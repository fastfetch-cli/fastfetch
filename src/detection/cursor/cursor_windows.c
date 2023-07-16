#include "cursor.h"

#include "util/windows/registry.h"

void ffDetectCursor(FFCursorResult* result)
{
    FF_HKEY_AUTO_DESTROY hKey;
    if(ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Control Panel\\Cursors", &hKey, &result->error))
    {
        if (!ffRegReadStrbuf(hKey, NULL, &result->theme, &result->error))
            return;

        uint32_t cursorBaseSize;
        if (ffRegReadUint(hKey, L"CursorBaseSize", &cursorBaseSize, NULL))
            ffStrbufAppendF(&result->size, "%u", (unsigned) cursorBaseSize);
    }
}
