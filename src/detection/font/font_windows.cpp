extern "C" {
#include "font.h"
#include "common/font.h"
}
#include "util/windows/wmi.hpp"

#include <wchar.h>

extern "C"
void ffDetectFontImpl(const FFinstance* instance, FFFontResult* result)
{
    wchar_t sql[256] = {};
    swprintf(sql, 256, L"SELECT IconTitleFaceName, IconTitleSize FROM Win32_Desktop WHERE Name LIKE '%%\\\\%s'", instance->state.passwd->pw_name);

    FFWmiQuery query(sql, &result->error);
    if(!query)
        return;

    if(FFWmiRecord record = query.next())
    {
        FFstrbuf fontName;
        ffStrbufInit(&fontName);
        record.getString(L"IconTitleFaceName", &fontName);

        uint64_t fontSize;
        record.getUnsigned(L"IconTitleSize", &fontSize);

        ffStrbufAppendF(&result->fonts[0], "%*s (%upt)", fontName.length, fontName.chars, (unsigned)fontSize);

        ffStrbufDestroy(&fontName);
    }
    else
        ffStrbufInitS(&result->error, "No WMI result returned");
}
