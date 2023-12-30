#include "com.hpp"
#include "fastfetch.h"

#include <stdlib.h>

//https://learn.microsoft.com/en-us/windows/win32/wmisdk/example--getting-wmi-data-from-the-local-computer
//https://learn.microsoft.com/en-us/windows/win32/cimwin32prov/computer-system-hardware-classes
static void CoUninitializeWrap(void)
{
    CoUninitialize();
}

static const char* doInitCom()
{
    // Initialize COM
    if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
        return "CoInitializeEx() failed";

    // Set general COM security levels
    if (FAILED(CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities
        NULL                         // Reserved
    )))
    {
        CoUninitialize();
        return "CoInitializeSecurity() failed";
    }

    atexit(CoUninitializeWrap);
    return NULL;
}

const char* ffInitCom(void)
{
    static const char* error = "";
    if (error && error[0] == '\0')
        error = doInitCom();
    return error;
}
