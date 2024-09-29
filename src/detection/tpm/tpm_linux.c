#include "tpm.h"
#include "common/io/io.h"

const char* ffDetectTPM(FFTPMResult* result)
{
    if (!ffPathExists("/sys/class/tpm/tpm0", FF_PATHTYPE_DIRECTORY))
    {
        if (!ffPathExists("/sys/class/tpm", FF_PATHTYPE_DIRECTORY))
            return "TPM is not supported by kernel";
        return "TPM device is not found";
    }

    if (ffReadFileBuffer("/sys/class/tpm/tpm0/tpm_version_major", &result->version))
    {
        ffStrbufTrimRightSpace(&result->version);
        if (ffStrbufEqualS(&result->version, "2"))
            ffStrbufSetStatic(&result->version, "2.0");
    }

    if (ffReadFileBuffer("/sys/class/tpm/tpm0/device/description", &result->description))
        ffStrbufTrimRightSpace(&result->description);

    return NULL;
}
