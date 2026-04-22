#include "swap.h"

#include "common/sysctl.h"
#include <mach/mach.h>

const char* ffDetectSwap(FFlist* result) {
    struct xsw_usage xsw;
    size_t size = sizeof(xsw);
    if (sysctl((int[]) { CTL_VM, VM_SWAPUSAGE }, 2, &xsw, &size, NULL, 0) != 0) {
        return "Failed to read vm.swapusage";
    }

    if (xsw.xsu_total == 0) {
        if (__builtin_available(macOS 26.0, *)) {
            // "vm.compressor_mode" no longer exists in macOS 26.0
        } else {
            if (ffSysctlGetInt("vm.compressor_mode", 4) <= 2) {
                return NULL; // Swap is disabled
            }
        }
    }

    FFSwapResult* swap = FF_LIST_ADD(FFSwapResult, *result);
    ffStrbufInitStatic(&swap->name, xsw.xsu_encrypted ? "Encrypted" : "Normal");
    swap->bytesTotal = xsw.xsu_total;
    swap->bytesUsed = xsw.xsu_used;
    return NULL;
}
