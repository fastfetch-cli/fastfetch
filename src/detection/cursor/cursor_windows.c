#include "cursor.h"

#include "util/windows/registry.h"

void ffDetectCursor(const FFinstance* instance, FFCursorResult* result)
{
    FF_UNUSED(instance);

    FF_HKEY_AUTO_DESTROY hKey;
    if(ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Control Panel\\Cursors", &hKey, &result->error))
        ffRegReadStrbuf(hKey, NULL, &result->theme, &result->error);
}
