#include "tpm.h"
#include "common/debug.h"
#include "common/sysctl.h"
#include "common/kmod.h"

const char* ffDetectTPM(FFTPMResult* result) {
    if (ffSysctlGetString("dev.tpmcrb.0.%desc", &result->description) != nullptr) {
        if (!ffKmodLoaded("tpm")) {
            FF_DEBUG("`tpm` kernel module is not loaded");
            return "`tpm` kernel module is not loaded";
        }
        FF_DEBUG("TPM device is not found");
        return "TPM device is not found";
    }

    if (ffStrbufContainS(&result->description, "2.0")) {
        ffStrbufSetStatic(&result->version, "2.0");
    } else if (ffStrbufContainS(&result->description, "1.2")) {
        ffStrbufSetStatic(&result->version, "1.2");
    } else {
        ffStrbufSetStatic(&result->version, "unknown");
    }

    return nullptr;
}
