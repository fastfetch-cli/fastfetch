#include "FFPlatform_private.h"
#include "util/stringUtils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj.h>

#include "util/windows/unicode.h"

static void getHomeDir(FFPlatform* platform)
{
    PWSTR pPath;
    if(SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_Profile, KF_FLAG_DEFAULT, NULL, &pPath)))
    {
        ffStrbufSetWS(&platform->homeDir, pPath);
        ffStrbufReplaceAllC(&platform->homeDir, '\\', '/');
        ffStrbufEnsureEndsWithC(&platform->homeDir, '/');
    }
    CoTaskMemFree(pPath);
}

static void getCacheDir(FFPlatform* platform)
{
    PWSTR pPath;
    if(SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_LocalAppData, KF_FLAG_DEFAULT, NULL, &pPath)))
    {
        ffStrbufSetWS(&platform->cacheDir, pPath);
        ffStrbufReplaceAllC(&platform->cacheDir, '\\', '/');
        ffStrbufEnsureEndsWithC(&platform->cacheDir, '/');
    }
    else
    {
        ffStrbufAppend(&platform->cacheDir, &platform->homeDir);
        ffStrbufAppendS(&platform->cacheDir, "AppData/Local/");
    }
    CoTaskMemFree(pPath);
}

static void platformPathAddKnownFolder(FFlist* dirs, REFKNOWNFOLDERID folderId)
{
    PWSTR pPath;
    if(SUCCEEDED(SHGetKnownFolderPath(folderId, 0, NULL, &pPath)))
    {
        FFstrbuf* buffer = (FFstrbuf*) ffListAdd(dirs);
        ffStrbufInit(buffer);
        ffStrbufSetWS(buffer, pPath);
        ffStrbufReplaceAllC(buffer, '\\', '/');
        ffStrbufEnsureEndsWithC(buffer, '/');
        FF_PLATFORM_PATH_UNIQUE(dirs, buffer);
    }
    CoTaskMemFree(pPath);
}

static void platformPathAddEnvSuffix(FFlist* dirs, const char* env, const char* suffix)
{
    const char* value = getenv(env);
    if(!ffStrSet(value))
        return;

    FFstrbuf* buffer = ffListAdd(dirs);
    ffStrbufInitA(buffer, 64);
    ffStrbufAppendS(buffer, value);
    ffStrbufReplaceAllC(buffer, '\\', '/');
    ffStrbufEnsureEndsWithC(buffer, '/');
    ffStrbufAppendS(buffer, suffix);
    ffStrbufEnsureEndsWithC(buffer, '/');
    FF_PLATFORM_PATH_UNIQUE(dirs, buffer);
}

static void getConfigDirs(FFPlatform* platform)
{
    if(getenv("MSYSTEM"))
    {
        // We are in MSYS2 / Git Bash
        platformPathAddEnvSuffix(&platform->configDirs, "HOME", ".config/");
        platformPathAddEnvSuffix(&platform->configDirs, "HOME", "");
        platformPathAddEnvSuffix(&platform->configDirs, "MINGW_PREFIX", "etc");
    }

    ffPlatformPathAddHome(&platform->configDirs, platform, ".config/");
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
        platformPathAddEnvSuffix(&platform->dataDirs, "HOME", "");
        platformPathAddEnvSuffix(&platform->dataDirs, "MINGW_PREFIX", "share");
    }
    ffPlatformPathAddHome(&platform->dataDirs, platform, ".local/share/");
    platformPathAddKnownFolder(&platform->dataDirs, &FOLDERID_RoamingAppData);
    platformPathAddKnownFolder(&platform->dataDirs, &FOLDERID_LocalAppData);
    ffPlatformPathAddHome(&platform->dataDirs, platform, "");
}

static void getUserName(FFPlatform* platform)
{
    ffStrbufEnsureFree(&platform->userName, 64);
    DWORD len = (DWORD) ffStrbufGetFree(&platform->userName);
    if(GetUserNameA(platform->userName.chars, &len))
        platform->userName.length = (uint32_t) len;
}

static void getHostName(FFPlatform* platform)
{
    ffStrbufEnsureFree(&platform->hostName, 64);
    DWORD len = (DWORD) ffStrbufGetFree(&platform->hostName);
    if(GetComputerNameExA(ComputerNameDnsHostname, platform->hostName.chars, &len))
        platform->hostName.length = (uint32_t) len;
}

static void getDomainName(FFPlatform* platform)
{
    ffStrbufEnsureFree(&platform->domainName, 64);
    DWORD len = (DWORD) ffStrbufGetFree(&platform->domainName);
    if(GetComputerNameExA(ComputerNameDnsDomain, platform->domainName.chars, &len))
        platform->domainName.length = (uint32_t) len;
}

static void getSystemName(FFPlatform* platform)
{
    ffStrbufAppendS(&platform->systemName, "Windows_NT");
}

static void getSystemReleaseAndVersion(FFPlatform* platform)
{
    HKEY hKey;
    if(RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return;

    DWORD bufSize;

    char currentVersion[32];

    {
        DWORD currentMajorVersionNumber;
        DWORD currentMinorVersionNumber;
        bufSize = sizeof(currentMajorVersionNumber);
        if(RegGetValueW(hKey, NULL, L"CurrentMajorVersionNumber", RRF_RT_REG_DWORD, NULL, &currentMajorVersionNumber, &bufSize) == ERROR_SUCCESS &&
            RegGetValueW(hKey, NULL, L"CurrentMinorVersionNumber", RRF_RT_REG_DWORD, NULL, &currentMinorVersionNumber, &bufSize) == ERROR_SUCCESS
        )
            snprintf(currentVersion, sizeof(currentVersion), "%u.%u", (unsigned)currentMajorVersionNumber, (unsigned)currentMinorVersionNumber);
        else
        {
            bufSize = sizeof(currentVersion);
            if(RegGetValueA(hKey, NULL, "CurrentVersion", RRF_RT_REG_SZ, NULL, currentVersion, &bufSize) != ERROR_SUCCESS)
                strcpy(currentVersion, "0.0");
        }
    }

    char currentBuildNumber[32];
    bufSize = sizeof(currentBuildNumber);
    if(RegGetValueA(hKey, NULL, "CurrentBuildNumber", RRF_RT_REG_SZ, NULL, currentBuildNumber, &bufSize) != ERROR_SUCCESS)
        strcpy(currentBuildNumber, "0");

    DWORD ubr;
    bufSize = sizeof(ubr);
    if(RegGetValueA(hKey, NULL, "UBR", RRF_RT_REG_DWORD, NULL, &ubr, &bufSize) != ERROR_SUCCESS || bufSize != sizeof(ubr))
        ubr = 0;

    ffStrbufAppendF(&platform->systemRelease, "%s.%s.%u", currentVersion, currentBuildNumber, (unsigned)ubr);

    ffStrbufEnsureFree(&platform->systemVersion, 256);
    bufSize = (DWORD) ffStrbufGetFree(&platform->systemVersion);
    if(RegGetValueA(hKey, NULL, "DisplayVersion", RRF_RT_REG_SZ, NULL, platform->systemVersion.chars, &bufSize) == ERROR_SUCCESS)
        platform->systemVersion.length = (uint32_t) bufSize;

    RegCloseKey(hKey);
}

static void getSystemArchitecture(FFPlatform* platform)
{
    SYSTEM_INFO sysInfo = {0};
    GetNativeSystemInfo(&sysInfo);

    switch(sysInfo.wProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_AMD64:
            ffStrbufAppendS(&platform->systemArchitecture, "x86_64");
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            ffStrbufAppendS(&platform->systemArchitecture, "ia64");
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            ffStrbufAppendS(&platform->systemArchitecture, "i686");
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            ffStrbufAppendS(&platform->systemArchitecture, "aarch64");
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            ffStrbufAppendS(&platform->systemArchitecture, "arm");
            break;
        case PROCESSOR_ARCHITECTURE_PPC:
            ffStrbufAppendS(&platform->systemArchitecture, "ppc");
            break;
        case PROCESSOR_ARCHITECTURE_MIPS:
            ffStrbufAppendS(&platform->systemArchitecture, "mips");
            break;
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
        default:
            break;
    }
}

void ffPlatformInitImpl(FFPlatform* platform)
{
    getHomeDir(platform);
    getCacheDir(platform);
    getConfigDirs(platform);
    getDataDirs(platform);

    getUserName(platform);
    getHostName(platform);
    getDomainName(platform);

    getSystemName(platform);
    getSystemReleaseAndVersion(platform);
    getSystemArchitecture(platform);
}
