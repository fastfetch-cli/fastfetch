#include "cursor.h"

#include "common/io.h"
#include "common/windows/registry.h"

void ffDetectCursor(FFCursorResult* result)
{
    FF_AUTO_CLOSE_FD HANDLE hKey = NULL;
    if(ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Control Panel\\Cursors", &hKey, &result->error))
    {
        uint32_t cursorBaseSize;
        if (ffRegReadValues(hKey, 2, (FFRegValueArg[]) {
            FF_ARG(result->theme, NULL),
            FF_ARG(cursorBaseSize, L"CursorBaseSize"),
        }, &result->error))
            ffStrbufAppendUInt(&result->size, cursorBaseSize);
    }
}
