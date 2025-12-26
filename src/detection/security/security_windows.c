#include "security.h"
#include "detection/tpm/tpm.h"
#include "util/windows/registry.h"

#include <windows.h>

const char* ffDetectSecurity(FFSecurityResult* result)
{
    ffStrbufInit(&result->tpmStatus);
    ffStrbufInit(&result->secureBootStatus);

    // Detect TPM
    FFTPMResult tpm = {
        .version = ffStrbufCreate(),
        .description = ffStrbufCreate()
    };
    const char* tpmError = ffDetectTPM(&tpm);
    if (tpmError == NULL)
    {
        if (tpm.version.length > 0)
            ffStrbufSetF(&result->tpmStatus, "TPM %s", tpm.version.chars);
        else
            ffStrbufSetStatic(&result->tpmStatus, "TPM OK");
    }
    else
    {
        ffStrbufSetStatic(&result->tpmStatus, "TPM Not Available");
    }
    ffStrbufDestroy(&tpm.version);
    ffStrbufDestroy(&tpm.description);

    // Detect Secure Boot directly from registry
    DWORD uefiSecureBootEnabled = 0, bufSize = sizeof(uefiSecureBootEnabled);
    if (RegGetValueW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\SecureBoot\\State", L"UEFISecureBootEnabled", RRF_RT_REG_DWORD, NULL, &uefiSecureBootEnabled, &bufSize) == ERROR_SUCCESS)
    {
        if (uefiSecureBootEnabled)
            ffStrbufSetStatic(&result->secureBootStatus, "Secure Boot Enabled");
        else
            ffStrbufSetStatic(&result->secureBootStatus, "Secure Boot Disabled");
    }
    else
    {
        ffStrbufSetStatic(&result->secureBootStatus, "Secure Boot Unknown");
    }

    return NULL;
}
