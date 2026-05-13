#include "FFPlatform_private.h"
#include "common/io.h"
#include "common/library.h"
#include "common/stringUtils.h"
#include "common/windows/unicode.h"
#include "common/windows/registry.h"
#include "common/windows/nt.h"

#include <stdalign.h>
#include <windows.h>
#include <shlobj.h>
#include <sddl.h>

#define SECURITY_WIN32 1 // For secext.h
#include <secext.h>

static void getExePath(FFPlatform* platform) {
    wchar_t exePathW[MAX_PATH];

    FF_AUTO_CLOSE_FD HANDLE hPath = CreateFileW(
        ffGetPeb()->ProcessParameters->ImagePathName.Buffer,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);
    if (hPath != INVALID_HANDLE_VALUE) {
        DWORD len = GetFinalPathNameByHandleW(hPath, exePathW, MAX_PATH, FILE_NAME_NORMALIZED);
        if (len > 0 && len < MAX_PATH) {
            ffStrbufSetNWS(&platform->exePath, len, exePathW);
            if (ffStrbufStartsWithS(&platform->exePath, "\\\\?\\")) {
                ffStrbufSubstrAfter(&platform->exePath, 3);
            }
        }
    }

    if (platform->exePath.length == 0) {
        PCUNICODE_STRING imagePathName = &ffGetPeb()->ProcessParameters->ImagePathName;
        ffStrbufSetNWS(&platform->exePath, imagePathName->Length / sizeof(wchar_t), imagePathName->Buffer);
    }

    ffStrbufReplaceAllC(&platform->exePath, '\\', '/');
}

static void getHomeDir(FFPlatform* platform) {
    PWSTR pPath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_Profile, KF_FLAG_DEFAULT, NULL, &pPath))) {
        ffStrbufSetWS(&platform->homeDir, pPath);
        ffStrbufReplaceAllC(&platform->homeDir, '\\', '/');
        ffStrbufEnsureEndsWithC(&platform->homeDir, '/');
    } else {
        ffStrbufSetS(&platform->homeDir, getenv("USERPROFILE"));
        ffStrbufReplaceAllC(&platform->homeDir, '\\', '/');
        ffStrbufEnsureEndsWithC(&platform->homeDir, '/');
    }
    CoTaskMemFree(pPath);
}

static void getCacheDir(FFPlatform* platform) {
    PWSTR pPath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_LocalAppData, KF_FLAG_DEFAULT, NULL, &pPath))) {
        ffStrbufSetWS(&platform->cacheDir, pPath);
        ffStrbufReplaceAllC(&platform->cacheDir, '\\', '/');
        ffStrbufEnsureEndsWithC(&platform->cacheDir, '/');
    } else {
        ffStrbufAppend(&platform->cacheDir, &platform->homeDir);
        ffStrbufAppendS(&platform->cacheDir, "AppData/Local/");
    }
    CoTaskMemFree(pPath);
}

static void platformPathAddKnownFolder(FFlist* dirs, REFKNOWNFOLDERID folderId) {
    PWSTR pPath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(folderId, KF_FLAG_DEFAULT, NULL, &pPath))) {
        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreateWS(pPath);
        CoTaskMemFree(pPath);
        ffStrbufReplaceAllC(&buffer, '\\', '/');
        ffStrbufEnsureEndsWithC(&buffer, '/');
        if (!FF_LIST_CONTAINS(*dirs, &buffer, ffStrbufEqual)) {
            ffStrbufInitMove(FF_LIST_ADD(FFstrbuf, *dirs), &buffer);
        }
    }
}

static void platformPathAddEnvSuffix(FFlist* dirs, const char* env, const char* suffix) {
    const char* value = getenv(env);
    if (!ffStrSet(value)) {
        return;
    }

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreateA(64);
    ffStrbufAppendS(&buffer, value);
    ffStrbufReplaceAllC(&buffer, '\\', '/');
    ffStrbufEnsureEndsWithC(&buffer, '/');
    if (suffix) {
        ffStrbufAppendS(&buffer, suffix);
        ffStrbufEnsureEndsWithC(&buffer, '/');
    }

    if (ffPathExists(buffer.chars, FF_PATHTYPE_DIRECTORY) && !FF_LIST_CONTAINS(*dirs, &buffer, ffStrbufEqual)) {
        ffStrbufInitMove(FF_LIST_ADD(FFstrbuf, *dirs), &buffer);
    }
}

static void getConfigDirs(FFPlatform* platform) {
    if (getenv("MSYSTEM")) {
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

static void getDataDirs(FFPlatform* platform) {
    if (getenv("MSYSTEM") && getenv("HOME")) {
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

static void getUserName(FFPlatform* platform) {
    wchar_t buffer[256];
    DWORD size = ARRAY_SIZE(buffer);
    if (GetUserNameExW(NameDisplay, buffer, &size)) {
        ffStrbufSetWS(&platform->fullUserName, buffer);
    }

    NTSYSAPI NTSTATUS NTAPI LsaGetUserName(
        _Outptr_ PLSA_UNICODE_STRING * UserName,
        _Outptr_opt_ PLSA_UNICODE_STRING * DomainName);
    PLSA_UNICODE_STRING userName = NULL;
    if (NT_SUCCESS(LsaGetUserName(&userName, NULL))) {
        ffStrbufSetNWS(&platform->userName, userName->Length / sizeof(wchar_t), userName->Buffer);
        RtlFreeUnicodeString(userName); // Required. userName.Buffer is allocated separately
        LsaFreeMemory(userName);
    } else {
        ffStrbufSetS(&platform->userName, getenv("USERNAME"));
    }

    alignas(TOKEN_USER) char buf[SECURITY_MAX_SID_SIZE + sizeof(TOKEN_USER)];
    if (NT_SUCCESS(NtQueryInformationToken(NtCurrentProcessToken(), TokenUser, buf, sizeof(buf), &size))) {
        TOKEN_USER* tokenUser = (TOKEN_USER*) buf;
        UNICODE_STRING sidString = { .Buffer = buffer, .Length = 0, .MaximumLength = sizeof(buffer) };
        if (NT_SUCCESS(RtlConvertSidToUnicodeString(&sidString, tokenUser->User.Sid, FALSE))) {
            ffStrbufSetNWS(&platform->sid, sidString.Length / sizeof(wchar_t), sidString.Buffer);
        }
    }
}

static void getHostName(FFPlatform* platform) {
    wchar_t buffer[256];
    DWORD len = ARRAY_SIZE(buffer);
    if (GetComputerNameExW(ComputerNameDnsHostname, buffer, &len) && len > 0) {
        ffStrbufSetNWS(&platform->hostName, len, buffer);
    } else {
        len = ARRAY_SIZE(buffer);
        if (GetComputerNameExW(ComputerNameNetBIOS, buffer, &len) && len > 0) {
            ffStrbufSetNWS(&platform->hostName, len, buffer);
        }
    }
}

static void getUserShell(FFPlatform* platform) {
    // Works in MSYS2
    const char* userShell = getenv("SHELL");
    if (userShell) {
        ffStrbufAppendS(&platform->userShell, userShell);
        ffStrbufReplaceAllC(&platform->userShell, '\\', '/');
    }
}

static const char* detectWine(void) {
    const char* __cdecl wine_get_version(void);
    void* hntdll = ffLibraryGetModule(L"ntdll.dll");
    if (!hntdll) {
        return NULL;
    }
    FF_LIBRARY_LOAD_SYMBOL_LAZY(hntdll, wine_get_version);
    if (!ffwine_get_version) {
        return NULL;
    }
    return ffwine_get_version();
}

static void getSystemReleaseAndVersion(FFPlatformSysinfo* info) {
    FF_AUTO_CLOSE_FD HANDLE hKey = NULL;
    if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", &hKey, NULL)) {
        return;
    }

    uint32_t ubr = 0;
    ffRegReadValues(hKey, 2, (FFRegValueArg[]) {
                                 FF_ARG(ubr, L"UBR"),
                                 FF_ARG(info->version, L"BuildLabEx"),
                             },
        NULL);

    PPEB_FULL peb = ffGetPeb();

    ffStrbufSetF(&info->release,
        "%u.%u.%u.%u",
        (unsigned) peb->OSMajorVersion,
        (unsigned) peb->OSMinorVersion,
        (unsigned) peb->OSBuildNumber,
        (unsigned) ubr);

    const char* wineVersion = detectWine();
    if (wineVersion) {
        ffStrbufSetF(&info->name, "Wine_%s", wineVersion);
    } else {
        ffStrbufSetStatic(&info->name, "WIN32_NT");
    }
}

static void getSystemPageSize(FFPlatformSysinfo* info) {
    SYSTEM_BASIC_INFORMATION sbi;
    if (NT_SUCCESS(NtQuerySystemInformation(SystemBasicInformation, &sbi, sizeof(sbi), NULL))) {
        info->pageSize = sbi.PhysicalPageSize;
    } else {
        info->pageSize = 4096;
    }
}

static void getSystemArchitecture(FFPlatformSysinfo* info) {
    SYSTEM_PROCESSOR_INFORMATION spi;
    if (NT_SUCCESS(NtQuerySystemInformation(SystemProcessorInformation, &spi, sizeof(spi), NULL))) {
        switch (spi.ProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_AMD64:
                ffStrbufSetStatic(&info->architecture, "x86_64");
                break;
            case PROCESSOR_ARCHITECTURE_IA64:
                ffStrbufSetStatic(&info->architecture, "ia64");
                break;
            case PROCESSOR_ARCHITECTURE_INTEL:
                switch (spi.ProcessorLevel) {
                    case 4:
                        ffStrbufSetStatic(&info->architecture, "i486");
                        break;
                    case 5:
                        ffStrbufSetStatic(&info->architecture, "i586");
                        break;
                    case 6:
                        ffStrbufSetStatic(&info->architecture, "i686");
                        break;
                    default:
                        ffStrbufSetStatic(&info->architecture, "i386");
                        break;
                }
                break;
            case PROCESSOR_ARCHITECTURE_ARM64:
                ffStrbufSetStatic(&info->architecture, "aarch64");
                break;
            case PROCESSOR_ARCHITECTURE_ARM:
                ffStrbufSetStatic(&info->architecture, "arm");
                break;
            case PROCESSOR_ARCHITECTURE_PPC:
                ffStrbufSetStatic(&info->architecture, "ppc");
                break;
            case PROCESSOR_ARCHITECTURE_MIPS:
                ffStrbufSetStatic(&info->architecture, "mips");
                break;
            case PROCESSOR_ARCHITECTURE_ALPHA:
                ffStrbufSetStatic(&info->architecture, "alpha");
                break;
            case PROCESSOR_ARCHITECTURE_ALPHA64:
                ffStrbufSetStatic(&info->architecture, "alpha64");
                break;
            case PROCESSOR_ARCHITECTURE_UNKNOWN:
            default:
                ffStrbufSetStatic(&info->architecture, "unknown");
                break;
        }
    }
}

static void getCwd(FFPlatform* platform) {
    PCURDIR cwd = &ffGetPeb()->ProcessParameters->CurrentDirectory;
    ffStrbufSetNWS(&platform->cwd, cwd->DosPath.Length / sizeof(WCHAR), cwd->DosPath.Buffer);
    ffStrbufReplaceAllC(&platform->cwd, '\\', '/');
    ffStrbufEnsureEndsWithC(&platform->cwd, '/');
}

void ffPlatformInitImpl(FFPlatform* platform) {
    platform->pid = (uint32_t) (uintptr_t) ffGetTeb()->ClientId.UniqueProcess;
    getExePath(platform);
    getCwd(platform);
    getHomeDir(platform);
    getCacheDir(platform);
    getConfigDirs(platform);
    getDataDirs(platform);

    getUserName(platform);
    getHostName(platform);
    getUserShell(platform);

    getSystemReleaseAndVersion(&platform->sysinfo);
    getSystemArchitecture(&platform->sysinfo);
    getSystemPageSize(&platform->sysinfo);
}
