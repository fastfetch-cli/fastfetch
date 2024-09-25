#include "tpm.h"
#include "common/io/io.h"

const char* ffDetectTPM(FF_MAYBE_UNUSED FFTPMResult* result)
{
    if (ffReadFileBuffer("/sys/class/tpm/tpm0/tpm_version_major", &result->version))
    {
        ffStrbufTrimRightSpace(&result->version);
        if (ffStrbufEqualS(&result->version, "2"))
            ffStrbufSetStatic(&result->version, "2.0");

        return NULL;
    }

    if (ffPathExists("/sys/class/tpm/tpm0", FF_PATHTYPE_DIRECTORY))
    {
        ffStrbufSetStatic(&result->version, "unknown");
        return NULL;
    }

    return "TPM device is not found";
}
