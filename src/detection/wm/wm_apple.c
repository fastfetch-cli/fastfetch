#include "wm.h"

#include "common/sysctl.h"
#include "util/mallocHelper.h"

#include <ctype.h>

const char* ffDetectWMPlugin(FFstrbuf* pluginName)
{
    int request[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    u_int requestLength = sizeof(request) / sizeof(*request);

    size_t length = 0;
    FF_AUTO_FREE struct kinfo_proc* processes = ffSysctlGetData(request, requestLength, &length);
    if(processes == NULL)
        return "sysctl(CTL_KERN, KERN_PROC, KERN_PROC_ALL) failed";

    for(size_t i = 0; i < length / sizeof(struct kinfo_proc); i++)
    {
        const char* comm = processes[i].kp_proc.p_comm;

        if(
            strcasecmp(comm, "spectacle") != 0 &&
            strcasecmp(comm, "amethyst") != 0 &&
            strcasecmp(comm, "kwm") != 0 &&
            strcasecmp(comm, "chunkwm") != 0 &&
            strcasecmp(comm, "yabai") != 0 &&
            strcasecmp(comm, "rectangle") != 0
        ) continue;

        ffStrbufAppendS(pluginName, comm);
        pluginName->chars[0] = (char) toupper(pluginName->chars[0]);
        break;
    }

    return NULL;
}
