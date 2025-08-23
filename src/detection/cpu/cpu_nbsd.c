#include "cpu.h"
#include "common/sysctl.h"
#include "common/io/io.h"

#include <sys/envsys.h>
#include <prop/proplib.h>
#include <paths.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

static void freePropDict(prop_dictionary_t* pdict)
{
    assert(pdict != NULL);
    if (*pdict == NULL) return;
    prop_object_release(*pdict);
}

static const char* detectCpuTemp(double* current)
{
    FF_AUTO_CLOSE_FD int fd = open(_PATH_SYSMON, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return "open(_PATH_SYSMON, O_RDONLY | O_CLOEXEC) failed";

    __attribute__((__cleanup__(freePropDict))) prop_dictionary_t root = NULL;
    if (prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &root) < 0)
        return "prop_dictionary_recv_ioctl(ENVSYS_GETDICTIONARY) failed";

    prop_array_t array = prop_dictionary_get(root, "coretemp0");
    if (!array) array = prop_dictionary_get(root, "amdzentemp0");
    if (!array) array = prop_dictionary_get(root, "viac7temp0");
    if (!array) array = prop_dictionary_get(root, "acpitz0"); // Thermal Zones
    if (!array) return "No temp data found in root dictionary";

    if (prop_array_count(array) != 2)
        return "Unexpected `xtemp0` data";

    prop_dictionary_t dict = prop_array_get(array, 0);
    if (prop_object_type(dict) != PROP_TYPE_DICTIONARY)
        return "Unexpected `xtemp0[0]`";

    int temp = 0; // in µK
    if (!prop_dictionary_get_int(dict, "cur-value", &temp))
        return "Failed to get temperature";

    *current = temp / 1e6 - 273.15;

    return NULL;
}

const char* ffDetectCPUImpl(const FFCPUOptions* options, FFCPUResult* cpu)
{
    if (ffSysctlGetString("machdep.cpu_brand", &cpu->name) != NULL &&
        ffSysctlGetString("machdep.dmi.processor-version", &cpu->name) != NULL &&
        ffSysctlGetString("hw.cpu0.name", &cpu->name) != NULL &&
        ffSysctlGetString("hw.model", &cpu->name) != NULL)
    {
        ffStrbufSetS(&cpu->name, "Unknown CPU");
    }

    if (ffSysctlGetString("machdep.dmi.processor-vendor", &cpu->vendor) == NULL)
        ffStrbufTrimRightSpace(&cpu->vendor);

    cpu->coresPhysical = (uint16_t) ffSysctlGetInt("hw.ncpu", 1);
    cpu->coresLogical = cpu->coresPhysical;
    cpu->coresOnline = (uint16_t) ffSysctlGetInt("hw.ncpuonline", cpu->coresLogical);

    ffCPUDetectByCpuid(cpu);

    uint32_t freq = (uint32_t) ffSysctlGetInt("machdep.cpu.frequency.target", 0);
    if (freq == 0) freq = (uint32_t) (ffSysctlGetInt64("hw.cpu0.clock_frequency", 0) / 1000000);
    if (freq == 0) freq = (uint32_t) ffSysctlGetInt("machdep.dmi.processor-frequency", 0);
    if (freq > cpu->frequencyBase) cpu->frequencyBase = freq;

    cpu->temperature = FF_CPU_TEMP_UNSET;

    if (options->temp) detectCpuTemp(&cpu->temperature);

    return NULL;
}
