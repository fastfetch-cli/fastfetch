#include "tpm.h"
#include "common/library.h"

#include <tbs.h>
#include <winerror.h>

const char* ffDetectTPM(FFTPMResult* result)
{
    FF_LIBRARY_LOAD(tbs, "dlopen TBS" FF_LIBRARY_EXTENSION " failed", "TBS" FF_LIBRARY_EXTENSION, -1)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(tbs, Tbsi_GetDeviceInfo)

    TPM_DEVICE_INFO deviceInfo = {};
    TBS_RESULT code = ffTbsi_GetDeviceInfo(sizeof(deviceInfo), &deviceInfo);
    if (code != TBS_SUCCESS)
        return code == (TBS_RESULT) TBS_E_TPM_NOT_FOUND ? "TPM device is not found" : "Tbsi_GetDeviceInfo() failed";

    switch (deviceInfo.tpmVersion)
    {
    case TPM_VERSION_12:
        ffStrbufSetStatic(&result->version, "1.2");
        break;
    case TPM_VERSION_20:
        ffStrbufSetStatic(&result->version, "2.0");
        break;
    default:
        ffStrbufSetStatic(&result->version, "unknown");
        break;
    }

    switch (deviceInfo.tpmInterfaceType)
    {
    case TPM_IFTYPE_1:
        ffStrbufSetF(&result->description, "I/O-port or MMIO TPM %s", result->version.chars);
        break;
    case TPM_IFTYPE_TRUSTZONE:
        ffStrbufSetF(&result->description, "Trustzone TPM %s", result->version.chars);
        break;
    case TPM_IFTYPE_HW:
        ffStrbufSetF(&result->description, "HW TPM %s", result->version.chars);
        break;
    case TPM_IFTYPE_EMULATOR:
        ffStrbufSetF(&result->description, "SW-emulator TPM %s", result->version.chars);
        break;
    case TPM_IFTYPE_SPB:
        ffStrbufSetF(&result->description, "SPB attached TPM %s", result->version.chars);
        break;
    default:
        break;
    }

    return NULL;
}
