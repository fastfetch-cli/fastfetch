#include "common/debug.h"
#include "common/mallocHelper.h"
#include "common/windows/version.h"
#include "common/windows/unicode.h"

#include <windows.h>

#define FF_VERSION_LANG_EN_US L"040904b0"

bool ffGetFileVersion(const wchar_t* filePath, const wchar_t* stringName, FFstrbuf* version)
{
    FF_DEBUG("ffGetFileVersion: enter filePath=%ls stringName=%ls", filePath, stringName);

    DWORD handle;
    DWORD size = GetFileVersionInfoSizeW(filePath, &handle);
    if (size == 0)
    {
        DWORD err = GetLastError();
        FF_DEBUG("GetFileVersionInfoSizeW failed: err=%lu (%s)",
            (unsigned long) err, ffDebugWin32Error(err));
        return false;
    }

    FF_DEBUG("GetFileVersionInfoSizeW ok: size=%lu handle=%lu",
        (unsigned long) size, (unsigned long) handle);

    FF_AUTO_FREE void* versionData = malloc(size);
    if (!versionData)
    {
        FF_DEBUG("malloc failed: size=%lu", (unsigned long) size);
        return false;
    }

    if (!GetFileVersionInfoW(filePath, handle, size, versionData))
    {
        DWORD err = GetLastError();
        FF_DEBUG("GetFileVersionInfoW failed: err=%lu (%s)",
            (unsigned long) err, ffDebugWin32Error(err));
        return false;
    }

    FF_DEBUG("GetFileVersionInfoW ok");

    if (!stringName)
    {
        VS_FIXEDFILEINFO* verInfo;
        UINT len;
        if (VerQueryValueW(versionData, L"\\", (void**) &verInfo, &len) &&
            len &&
            verInfo->dwSignature == 0xFEEF04BD)
        {
            ffStrbufSetF(version, "%u.%u.%u.%u",
                (unsigned) ((verInfo->dwProductVersionMS >> 16) & 0xffff),
                (unsigned) ((verInfo->dwProductVersionMS >> 0) & 0xffff),
                (unsigned) ((verInfo->dwProductVersionLS >> 16) & 0xffff),
                (unsigned) ((verInfo->dwProductVersionLS >> 0) & 0xffff));
            FF_DEBUG("fixed version resolved: %s", version->chars);
            return true;
        }

        FF_DEBUG("fixed version query failed or invalid signature");
        return false;
    }

    wchar_t* value;
    UINT valueLen; // Number of characters, including null terminator

    wchar_t subBlock[128];
    snwprintf(subBlock, ARRAY_SIZE(subBlock), L"\\StringFileInfo\\" FF_VERSION_LANG_EN_US L"\\%ls", stringName);
    FF_DEBUG("query version string with default lang (en_US): %ls", subBlock);

    if (VerQueryValueW(versionData, subBlock, (void**) &value, &valueLen) && valueLen > 0)
    {
        ffStrbufSetNWS(version, valueLen - 1, value);
        FF_DEBUG("version string resolved (default lang): %s", version->chars);
        return true;
    }

    FF_DEBUG("default lang query failed, trying translation fallback");

    struct { WORD language; WORD codePage; }* translations;
    UINT translationsLen;

    if (VerQueryValueW(versionData, L"\\VarFileInfo\\Translation", (void**) &translations, &translationsLen) &&
        translationsLen >= sizeof(*translations))
    {
        snwprintf(subBlock, ARRAY_SIZE(subBlock), L"\\StringFileInfo\\%04x%04x\\%ls",
                  translations[0].language, translations[0].codePage, stringName);
        FF_DEBUG("query version string with translation: %ls", subBlock);

        if (VerQueryValueW(versionData, subBlock, (void**) &value, &valueLen) && valueLen > 0)
        {
            ffStrbufSetNWS(version, valueLen - 1, value);
            FF_DEBUG("version string resolved (translation fallback): %s", version->chars);
            return true;
        }

        FF_DEBUG("translation fallback query failed");
    }
    else
    {
        FF_DEBUG("no translation table found in version resource");
    }

    FF_DEBUG("ffGetFileVersion failed");
    return false;
}
