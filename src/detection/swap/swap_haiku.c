#include "swap.h"

#include <OS.h>
#include <driver_settings.h>

const char* ffDetectSwap(FFlist* result)
{
    system_info info;
    if (get_system_info(&info) != B_OK)
        return "Error getting system info";

    uint32_t pageSize = instance.state.platform.sysinfo.pageSize;
    FFSwapResult* swap = ffListAdd(result);
    ffStrbufInitStatic(&swap->name, "System");
    void* kvms = load_driver_settings("virtual_memory"); // /boot/home/config/settings/kernel/drivers/virtual_memory
    if (kvms)
    {
        const char* swapAuto = get_driver_parameter(kvms, "swap_auto", NULL, NULL);
        if (swapAuto)
            ffStrbufSetStatic(&swap->name, swapAuto[0] == 'y' ? "Auto" : "Manual");
        unload_driver_settings(kvms);
    }
    swap->bytesTotal = pageSize * (uint64_t) info.max_swap_pages;
    swap->bytesUsed = pageSize * (uint64_t) (info.max_swap_pages - info.free_swap_pages);

    return NULL;
}
