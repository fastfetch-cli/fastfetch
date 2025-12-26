#include "security.h"
#include "detection/tpm/tpm.h"
#include "detection/bootmgr/bootmgr.h"

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

    // Detect Secure Boot
    FFBootmgrResult bootmgr = {
        .name = ffStrbufCreate(),
        .firmware = ffStrbufCreate(),
        .order = 0,
        .secureBoot = false
    };
    const char* bootmgrError = ffDetectBootmgr(&bootmgr);
    if (bootmgrError == NULL)
    {
        if (bootmgr.secureBoot)
            ffStrbufSetStatic(&result->secureBootStatus, "Secure Boot Enabled");
        else
            ffStrbufSetStatic(&result->secureBootStatus, "Secure Boot Disabled");
    }
    else
    {
        ffStrbufSetStatic(&result->secureBootStatus, "Secure Boot Unknown");
    }
    ffStrbufDestroy(&bootmgr.name);
    ffStrbufDestroy(&bootmgr.firmware);

    return NULL;
}
