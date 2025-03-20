#include "util/windows/version.h"
#include "util/mallocHelper.h"
#include "util/windows/unicode.h"

#include <windows.h>

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
                    ffStrbufAppendF(version, "%u.%u.%u.%u",
                                    (unsigned)((verInfo->dwProductVersionMS >> 16) & 0xffff),
                                    (unsigned)((verInfo->dwProductVersionMS >> 0) & 0xffff),
                                    (unsigned)((verInfo->dwProductVersionLS >> 16) & 0xffff),
                                    (unsigned)((verInfo->dwProductVersionLS >> 0) & 0xffff));
                    return true;
                }
            }
            else
            {
                struct
                {
                    WORD language;
                    WORD codePage;
                }* translations;

                UINT translationsLen;

                if (VerQueryValueW(versionData, L"\\VarFileInfo\\Translation",
                                   (void **) &translations, &translationsLen) &&
                    translationsLen >= sizeof(*translations))
                {
                    wchar_t subBlock[128];
                    snwprintf(subBlock, ARRAY_SIZE(subBlock), L"\\StringFileInfo\\%04x%04x\\%ls",
                              translations[0].language, translations[0].codePage, stringName);

                    wchar_t* value;
                    UINT valueLen;

                    if (VerQueryValueW(versionData, subBlock, (void **)&value, &valueLen) && valueLen > 0)
                    {
                        ffStrbufSetWS(version, value);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
