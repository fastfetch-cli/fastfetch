#include "tpm.h"
#include "common/sysctl.h"

#include <sys/param.h>
#include <sys/module.h>

const char* ffDetectTPM(FFTPMResult* result)
{
    if (ffSysctlGetString("dev.tpmcrb.0.%desc", &result->description) != NULL)
    {
        if (modfind("tpm") < 0) return "TPM kernel module is not loaded";
        return "TPM device is not found";
    }

    if (ffStrbufContainS(&result->description, "2.0"))
        ffStrbufSetStatic(&result->version, "2.0");
    else if (ffStrbufContainS(&result->description, "1.2"))
        ffStrbufSetStatic(&result->version, "1.2");
    else
        ffStrbufSetStatic(&result->version, "unknown");

    return "Not supported on this platform";
}
