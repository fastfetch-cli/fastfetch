#include "fastfetch.h"
#include "utsname.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

static int detectSysname(struct utsname *name)
{
    strncpy(name->sysname, "Windows_NT", UTSNAME_MAXLENGTH);
    return 0;
}

static int detectNodename(struct utsname *name)
{
    DWORD bufSize = UTSNAME_MAXLENGTH - 1;
    if(!GetComputerNameExA(ComputerNameDnsFullyQualified, name->nodename, &bufSize))
        return 1;
    name->nodename[bufSize] = '\0';
    return 0;
}

static int detectVersion(struct utsname *name)
{
    HKEY hKey;
    if(RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return 1;

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

    snprintf(name->release, sizeof(name->release), "%s.%s.%u", currentVersion, currentBuildNumber, (unsigned)ubr);

    bufSize = sizeof(name->version);
    RegGetValueA(hKey, NULL, "DisplayVersion", RRF_RT_REG_SZ, NULL, name->version, &bufSize);

    RegCloseKey(hKey);
    return 0;
}

static int detectMachine(struct utsname *name)
{
    // Get hardware info
    SYSTEM_INFO sysInfo = {0};
    GetNativeSystemInfo(&sysInfo);

    // Set processor architecture
    switch(sysInfo.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64:
        strcpy(name->machine, "x86_64");
        break;
    case PROCESSOR_ARCHITECTURE_IA64:
        strcpy(name->machine, "ia64");
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        strcpy(name->machine, "x86");
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        strcpy(name->machine, "aarch64");
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        strcpy(name->machine, "arm");
        break;
    case PROCESSOR_ARCHITECTURE_PPC:
        strcpy(name->machine, "ppc");
        break;
    case PROCESSOR_ARCHITECTURE_MIPS:
        strcpy(name->machine, "mips");
        break;
    case PROCESSOR_ARCHITECTURE_UNKNOWN:
    default:
        strcpy(name->machine, "unknown");
        break;
    }

    return 0;
}

int uname(struct utsname *name)
{
    memset(name, 0, sizeof(*name));

    int sysnameResult = detectSysname(name);
    int nodenameResult = detectNodename(name);
    int versionResult = detectVersion(name);
    int machineResult = detectMachine(name);

    return sysnameResult || nodenameResult || versionResult || machineResult;
}
