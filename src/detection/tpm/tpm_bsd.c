#include "tpm.h"
#include "common/sysctl.h"
#include "util/kmod.h"

const char* ffDetectTPM(FFTPMResult* result)
{
    if (ffSysctlGetString("dev.tpmcrb.0.%desc", &result->description) != NULL)
    {
        if (!ffKmodLoaded("tpm")) return "`tpm` kernel module is not loaded";
        return "TPM device is not found";
    }

    if (ffStrbufContainS(&result->description, "2.0"))
        ffStrbufSetStatic(&result->version, "2.0");
    else if (ffStrbufContainS(&result->description, "1.2"))
        ffStrbufSetStatic(&result->version, "1.2");
    else
        ffStrbufSetStatic(&result->version, "unknown");

    return NULL;
}
