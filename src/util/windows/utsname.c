// https://github.com/tniessen/iperf-windows/blob/master/win32-compat/sys/utsname.c

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>

#include "utsname.h"

int uname(struct utsname *name)
{
    memset(name, 0, sizeof(*name));

    // Get Windows version info
    OSVERSIONINFOA versionInfo = {
        .dwOSVersionInfoSize = sizeof(OSVERSIONINFO),
    };
    GetVersionExA(&versionInfo);

    // Get hardware info
    SYSTEM_INFO sysInfo = {0};
    GetSystemInfo(&sysInfo);

    // Set implementation name
    strcpy(name->sysname, "Windows_NT");
    sprintf(name->release, "%u.%u.%u", (unsigned)versionInfo.dwMajorVersion, (unsigned)versionInfo.dwMinorVersion, (unsigned)versionInfo.dwBuildNumber);
    name->version[0] = '\0';

    // Set hostname
    DWORD bufSize = UTSNAME_MAXLENGTH - 1;
    if(GetComputerNameA(name->nodename, &bufSize))
        return 1;
    name->nodename[bufSize] = '\0';

    // Set processor architecture
    switch (sysInfo.wProcessorArchitecture)
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
    case PROCESSOR_ARCHITECTURE_UNKNOWN:
    default:
        strcpy(name->machine, "unknown");
    }

    return 0;
}
