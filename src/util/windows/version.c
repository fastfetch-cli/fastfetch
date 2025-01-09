#include "util/windows/version.h"
#include "util/mallocHelper.h"

#include <windows.h>

bool ffGetFileVersion(const wchar_t* filePath, FFstrbuf* version)
{
    DWORD handle;
    DWORD size = GetFileVersionInfoSizeW(filePath, &handle);
    if(size > 0)
    {
        FF_AUTO_FREE void* versionData = malloc(size);
        if(GetFileVersionInfoW(filePath, handle, size, versionData))
        {
            VS_FIXEDFILEINFO* verInfo;
            UINT len;
            if(VerQueryValueW(versionData, L"\\", (void**)&verInfo, &len) && len && verInfo->dwSignature == 0xFEEF04BD)
            {
                ffStrbufAppendF(version, "%u.%u.%u.%u",
                    (unsigned)(( verInfo->dwProductVersionMS >> 16 ) & 0xffff),
                    (unsigned)(( verInfo->dwProductVersionMS >>  0 ) & 0xffff),
                    (unsigned)(( verInfo->dwProductVersionLS >> 16 ) & 0xffff),
                    (unsigned)(( verInfo->dwProductVersionLS >>  0 ) & 0xffff)
                );
                return true;
            }
        }
    }

    return false;
}
