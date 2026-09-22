#include "swap.h"

#include <OS.h>
#include <driver_settings.h>

const char* ffDetectSwap(FFlist* result) {
    system_info info;
    if (get_system_info(&info) != B_OK) {
        return "Error getting system info";
    }

    const uint32_t pageSizeShift = instance.state.platform.sysinfo.pageSizeShift;
    FFSwapResult* swap = FF_LIST_ADD(FFSwapResult, *result);
    ffStrbufInitStatic(&swap->name, "System");
    void* kvms = load_driver_settings("virtual_memory"); // /boot/home/config/settings/kernel/drivers/virtual_memory
    if (kvms) {
        const char* swapAuto = get_driver_parameter(kvms, "swap_auto", nullptr, nullptr);
        if (swapAuto) {
            ffStrbufSetStatic(&swap->name, swapAuto[0] == 'y' ? "Auto" : "Manual");
        }
        unload_driver_settings(kvms);
    }
    swap->bytesTotal = (uint64_t) info.max_swap_pages << pageSizeShift;
    swap->bytesUsed = (uint64_t) (info.max_swap_pages - info.free_swap_pages) << pageSizeShift;

    return nullptr;
}
