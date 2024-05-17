#include "FFPlatform_private.h"
#include "common/io/io.h"
#include "util/stringUtils.h"
#include "util/windows/unicode.h"
#include "util/windows/registry.h"
#include "util/windows/nt.h"

#include <Windows.h>
#include <shlobj.h>

static void getExePath(FFPlatform* platform)
{
    wchar_t exePathW[MAX_PATH];
    DWORD exePathWLen = GetModuleFileNameW(NULL, exePathW, MAX_PATH);
    if (exePathWLen == 0 && exePathWLen >= MAX_PATH) return;

    FF_AUTO_CLOSE_FD HANDLE hPath = CreateFileW(exePathW, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hPath != INVALID_HANDLE_VALUE)
    {
        DWORD len = GetFinalPathNameByHandleW(hPath, exePathW, MAX_PATH, FILE_NAME_OPENED);
        if (len > 0 && len < MAX_PATH)
            exePathWLen = len;
    }

    ffStrbufSetNWS(&platform->exePath, exePathWLen, exePathW);
    if (ffStrbufStartsWithS(&platform->exePath, "\\\\?\\"))
        ffStrbufSubstrAfter(&platform->exePath, 3);
    ffStrbufReplaceAllC(&platform->exePath, '\\', '/');
}

static void getHomeDir(FFPlatform* platform)
{
    PWSTR pPath;
    if(SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_Profile, KF_FLAG_DEFAULT, NULL, &pPath)))
    {
        ffStrbufSetWS(&platform->homeDir, pPath);
        ffStrbufReplaceAllC(&platform->homeDir, '\\', '/');
        ffStrbufEnsureEndsWithC(&platform->homeDir, '/');
        CoTaskMemFree(pPath);
    }
    else
    {
        ffStrbufSetS(&platform->homeDir, getenv("USERPROFILE"));
        ffStrbufReplaceAllC(&platform->homeDir, '\\', '/');
        ffStrbufEnsureEndsWithC(&platform->homeDir, '/');
    }
}

static void getCacheDir(FFPlatform* platform)
{
    PWSTR pPath;
    if(SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_LocalAppData, KF_FLAG_DEFAULT, NULL, &pPath)))
    {
        ffStrbufSetWS(&platform->cacheDir, pPath);
        ffStrbufReplaceAllC(&platform->cacheDir, '\\', '/');
        ffStrbufEnsureEndsWithC(&platform->cacheDir, '/');
        CoTaskMemFree(pPath);
    }
    else
    {
        ffStrbufAppend(&platform->cacheDir, &platform->homeDir);
        ffStrbufAppendS(&platform->cacheDir, "AppData/Local/");
    }
}

static void platformPathAddKnownFolder(FFlist* dirs, REFKNOWNFOLDERID folderId)
{
    PWSTR pPath;
    if(SUCCEEDED(SHGetKnownFolderPath(folderId, 0, NULL, &pPath)))
    {
        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreateWS(pPath);
        ffStrbufReplaceAllC(&buffer, '\\', '/');
        ffStrbufEnsureEndsWithC(&buffer, '/');
        if (!ffListContains(dirs, &buffer, (void*) ffStrbufEqual))
            ffStrbufInitMove((FFstrbuf*) ffListAdd(dirs), &buffer);
        CoTaskMemFree(pPath);
    }
}

static void platformPathAddEnvSuffix(FFlist* dirs, const char* env, const char* suffix)
{
    const char* value = getenv(env);
    if(!ffStrSet(value))
        return;

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreateA(64);
    ffStrbufAppendS(&buffer, value);
    ffStrbufReplaceAllC(&buffer, '\\', '/');
    ffStrbufEnsureEndsWithC(&buffer, '/');
    if (suffix)
    {
        ffStrbufAppendS(&buffer, suffix);
        ffStrbufEnsureEndsWithC(&buffer, '/');
    }

    if (ffPathExists(buffer.chars, FF_PATHTYPE_DIRECTORY) && !ffListContains(dirs, &buffer, (void*) ffStrbufEqual))
        ffStrbufInitMove((FFstrbuf*) ffListAdd(dirs), &buffer);
}

static void getConfigDirs(FFPlatform* platform)
{
    if(getenv("MSYSTEM"))
    {
        // We are in MSYS2 / Git Bash
        platformPathAddEnvSuffix(&platform->configDirs, "HOME", ".config/");
        platformPathAddEnvSuffix(&platform->configDirs, "HOME", NULL);
        platformPathAddEnvSuffix(&platform->configDirs, "MINGW_PREFIX", "etc");
    }

    ffPlatformPathAddHome(&platform->configDirs, platform, ".config/");
    platformPathAddKnownFolder(&platform->configDirs, &FOLDERID_ProgramData);
    platformPathAddKnownFolder(&platform->configDirs, &FOLDERID_RoamingAppData);
    platformPathAddKnownFolder(&platform->configDirs, &FOLDERID_LocalAppData);
    ffPlatformPathAddHome(&platform->configDirs, platform, "");
}

static void getDataDirs(FFPlatform* platform)
{
    if(getenv("MSYSTEM") && getenv("HOME"))
    {
        // We are in MSYS2 / Git Bash
        platformPathAddEnvSuffix(&platform->dataDirs, "HOME", ".local/share/");
        platformPathAddEnvSuffix(&platform->dataDirs, "HOME", NULL);
        platformPathAddEnvSuffix(&platform->dataDirs, "MINGW_PREFIX", "share");
    }
    ffPlatformPathAddHome(&platform->dataDirs, platform, ".local/share/");
    platformPathAddKnownFolder(&platform->dataDirs, &FOLDERID_ProgramData);
    platformPathAddKnownFolder(&platform->dataDirs, &FOLDERID_RoamingAppData);
    platformPathAddKnownFolder(&platform->dataDirs, &FOLDERID_LocalAppData);
    ffPlatformPathAddHome(&platform->dataDirs, platform, "");
}

static void getUserName(FFPlatform* platform)
{
    const char* userName = getenv("USERNAME");
    if (ffStrSet(userName))
        ffStrbufSetS(&platform->userName, userName);
    else
    {
        wchar_t buffer[128];
        DWORD len = sizeof(buffer) / sizeof(*buffer);
        if(GetUserNameW(buffer, &len))
            ffStrbufSetWS(&platform->userName, buffer);
    }
}

static void getHostName(FFPlatform* platform)
{
    wchar_t buffer[128];
    DWORD len = sizeof(buffer) / sizeof(*buffer);
    if(GetComputerNameExW(ComputerNameDnsHostname, buffer, &len))
        ffStrbufSetWS(&platform->hostName, buffer);
}

static void getUserShell(FFPlatform* platform)
{
    // Works in MSYS2
    ffStrbufAppendS(&platform->userShell, getenv("SHELL"));
    ffStrbufReplaceAllC(&platform->userShell, '\\', '/');
}

static void getSystemReleaseAndVersion(FFPlatform* platform)
{
    RTL_OSVERSIONINFOW osVersion = { .dwOSVersionInfoSize = sizeof(osVersion) };
    if (!NT_SUCCESS(RtlGetVersion(&osVersion)))
        return;

    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", &hKey, NULL))
        return;

    uint32_t ubr = 0;
    ffRegReadUint(hKey, L"UBR", &ubr, NULL);

    ffStrbufAppendF(&platform->systemRelease,
        "%u.%u.%u.%u",
        (unsigned) osVersion.dwMajorVersion,
        (unsigned) osVersion.dwMinorVersion,
        (unsigned) osVersion.dwBuildNumber,
        (unsigned) ubr);

    ffStrbufInit(&platform->systemDisplayVersion);
    if(!ffRegReadStrbuf(hKey, L"DisplayVersion", &platform->systemDisplayVersion, NULL))
    {
        if (osVersion.szCSDVersion[0])
            ffStrbufSetWS(&platform->systemDisplayVersion, osVersion.szCSDVersion);
        else
            ffRegReadStrbuf(hKey, L"ReleaseId", &platform->systemDisplayVersion, NULL); // For old Windows 10
    }

    ffRegReadStrbuf(hKey, L"BuildLabEx", &platform->systemVersion, NULL);

    switch (osVersion.dwPlatformId)
    {
    case VER_PLATFORM_WIN32s:
        ffStrbufSetStatic(&platform->systemName, "WIN32s");
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        ffStrbufSetStatic(&platform->systemName, "WIN32_WINDOWS");
        break;
    case VER_PLATFORM_WIN32_NT:
        ffStrbufSetStatic(&platform->systemName, "WIN32_NT");
        break;
    }
}

static void getSystemArchitectureAndPageSize(FFPlatform* platform)
{
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);

    switch(sysInfo.wProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_AMD64:
            ffStrbufSetStatic(&platform->systemArchitecture, "x86_64");
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            ffStrbufSetStatic(&platform->systemArchitecture, "ia64");
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            switch (sysInfo.wProcessorLevel)
            {
                case 4:
                    ffStrbufSetStatic(&platform->systemArchitecture, "i486");
                    break;
                case 5:
                    ffStrbufSetStatic(&platform->systemArchitecture, "i586");
                    break;
                case 6:
                    ffStrbufSetStatic(&platform->systemArchitecture, "i686");
                    break;
                default:
                    ffStrbufSetStatic(&platform->systemArchitecture, "i386");
                    break;
            }
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            ffStrbufSetStatic(&platform->systemArchitecture, "aarch64");
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            ffStrbufSetStatic(&platform->systemArchitecture, "arm");
            break;
        case PROCESSOR_ARCHITECTURE_PPC:
            ffStrbufSetStatic(&platform->systemArchitecture, "ppc");
            break;
        case PROCESSOR_ARCHITECTURE_MIPS:
            ffStrbufSetStatic(&platform->systemArchitecture, "mips");
            break;
        case PROCESSOR_ARCHITECTURE_ALPHA:
            ffStrbufSetStatic(&platform->systemArchitecture, "alpha");
            break;
        case PROCESSOR_ARCHITECTURE_ALPHA64:
            ffStrbufSetStatic(&platform->systemArchitecture, "alpha64");
            break;
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
        default:
            break;
    }

    platform->pageSize = sysInfo.dwPageSize;
}

void ffPlatformInitImpl(FFPlatform* platform)
{
    getExePath(platform);
    getHomeDir(platform);
    getCacheDir(platform);
    getConfigDirs(platform);
    getDataDirs(platform);

    getUserName(platform);
    getHostName(platform);
    getUserShell(platform);

    getSystemReleaseAndVersion(platform);
    getSystemArchitectureAndPageSize(platform);
}
