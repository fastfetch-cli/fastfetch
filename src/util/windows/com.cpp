#include "com.hpp"
#include "fastfetch.h"

#include <stdlib.h>
#include <synchapi.h>

//https://learn.microsoft.com/en-us/windows/win32/wmisdk/example--getting-wmi-data-from-the-local-computer
//https://learn.microsoft.com/en-us/windows/win32/cimwin32prov/computer-system-hardware-classes
static void CoUninitializeWrap()
{
    CoUninitialize();
}

static BOOL CALLBACK InitHandleFunction(FF_MAYBE_UNUSED PINIT_ONCE lpInitOnce, FF_MAYBE_UNUSED PVOID lpParameter, PVOID* lpContext)
{
    // Initialize COM
    if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        *lpContext = (PVOID) "CoInitializeEx() failed";
        return FALSE;
    }

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
        *lpContext = (PVOID) "CoInitializeSecurity() failed";
        return FALSE;
    }

    atexit(CoUninitializeWrap);
    return TRUE;
}

const char* ffInitCom(void)
{
    const char* error = NULL;
    static INIT_ONCE s_InitOnce;
    InitOnceExecuteOnce(&s_InitOnce, &InitHandleFunction, NULL, (void**)&error);
    return error;
}
