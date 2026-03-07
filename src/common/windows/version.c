#include "common/windows/version.h"
#include "common/mallocHelper.h"
#include "common/windows/unicode.h"

#include <windows.h>

#define FF_VERSION_LANG_EN_US L"040904b0"

bool ffGetFileVersion(const wchar_t* filePath, const wchar_t* stringName, FFstrbuf* version)
{
    DWORD handle;
    DWORD size = GetFileVersionInfoSizeW(filePath, &handle);
    if (size > 0)
    {
        FF_AUTO_FREE void *versionData = malloc(size);
        if (GetFileVersionInfoW(filePath, handle, size, versionData))
        {
            if (!stringName)
            {
                VS_FIXEDFILEINFO* verInfo;
                UINT len;
                if (VerQueryValueW(versionData, L"\\", (void **)&verInfo, &len) && len && verInfo->dwSignature == 0xFEEF04BD)
                {
                    ffStrbufSetF(version, "%u.%u.%u.%u",
                        (unsigned)((verInfo->dwProductVersionMS >> 16) & 0xffff),
                        (unsigned)((verInfo->dwProductVersionMS >> 0) & 0xffff),
                        (unsigned)((verInfo->dwProductVersionLS >> 16) & 0xffff),
                        (unsigned)((verInfo->dwProductVersionLS >> 0) & 0xffff));
                    return true;
                }
            }
            else
            {
                wchar_t* value;
                UINT valueLen;

                wchar_t subBlock[128] = L"\\StringFileInfo\\" FF_VERSION_LANG_EN_US L"\\";
                wcscat_s(subBlock, ARRAY_SIZE(subBlock), stringName);
                if (VerQueryValueW(versionData, subBlock, (void **)&value, &valueLen) && valueLen > 0)
                {
                    ffStrbufSetWS(version, value);
                    return true;
                }
            }
        }
    }

    return false;
}
