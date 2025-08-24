#include "cpu.h"
#include "util/mallocHelper.h"

#include <OS.h>
#include <private/shared/cpu_type.h>

const char* ffDetectCPUImpl(FF_MAYBE_UNUSED const FFCPUOptions* options, FFCPUResult* cpu)
{
    system_info sysInfo;
    if (get_system_info(&sysInfo) != B_OK)
        return "get_system_info() failed";

    uint32 topoNodeCount = 0;
    get_cpu_topology_info(NULL, &topoNodeCount);
    if (topoNodeCount == 0)
        return "get_cpu_topology_info(NULL) failed";

    FF_AUTO_FREE cpu_topology_node_info* topology = malloc(sizeof(*topology) * topoNodeCount);
    if (get_cpu_topology_info(topology, &topoNodeCount) != B_OK)
        return "get_cpu_topology_info(topology) failed";

    enum cpu_platform platform = B_CPU_UNKNOWN;
    enum cpu_vendor cpuVendor = B_CPU_VENDOR_UNKNOWN;
    uint32 cpuModel = 0, frequency = 0;
    uint16_t packages = 0, cores = 0;

    for (uint32 i = 0; i < topoNodeCount; i++)
    {
        switch (topology[i].type) {
            case B_TOPOLOGY_ROOT:
                platform = topology[i].data.root.platform;
                break;

            case B_TOPOLOGY_PACKAGE:
                cpuVendor = topology[i].data.package.vendor;
                ++packages;
                break;

            case B_TOPOLOGY_CORE:
                cpuModel = topology[i].data.core.model;
                uint32_t freq = (uint32_t) (topology[i].data.core.default_frequency / 1000000);
                frequency = freq > frequency ? freq : frequency;
                ++cores;
                break;

            default:
                break;
        }
    }

    const char *model = get_cpu_model_string(platform, cpuVendor, cpuModel);
    if (model)
        ffStrbufSetS(&cpu->name, model);
    else
        ffStrbufSetF(&cpu->name, "(Unknown %" B_PRIx32 ")", cpuModel);
    ffStrbufSetS(&cpu->vendor, get_cpu_vendor_string(cpuVendor));

    ffCPUDetectByCpuid(cpu);
    if (cpu->frequencyBase < frequency) cpu->frequencyBase = frequency;
    cpu->packages = packages;
    cpu->coresPhysical = cores;
    cpu->coresOnline = cpu->coresLogical = (uint16_t) sysInfo.cpu_count;
    return NULL;
}
