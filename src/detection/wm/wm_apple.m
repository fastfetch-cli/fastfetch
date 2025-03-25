#include "wm.h"

#include "common/sysctl.h"
#include "util/mallocHelper.h"
#include "util/stringUtils.h"

#include <ctype.h>
#import <Foundation/Foundation.h>

const char* ffDetectWMPlugin(FFstrbuf* pluginName)
{
    int request[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    u_int requestLength = ARRAY_SIZE(request);

    size_t length = 0;
    FF_AUTO_FREE struct kinfo_proc* processes = ffSysctlGetData(request, requestLength, &length);
    if(processes == NULL)
        return "sysctl(CTL_KERN, KERN_PROC, KERN_PROC_ALL) failed";
    assert(length % sizeof(struct kinfo_proc) == 0);

    for(size_t i = 0; i < length / sizeof(struct kinfo_proc); i++)
    {
        if (processes[i].kp_eproc.e_ppid != 1) continue;

        const char* comm = processes[i].kp_proc.p_comm;

        if(
            !ffStrEqualsIgnCase(comm, "spectacle") &&
            !ffStrEqualsIgnCase(comm, "amethyst") &&
            !ffStrEqualsIgnCase(comm, "kwm") &&
            !ffStrEqualsIgnCase(comm, "chunkwm") &&
            !ffStrEqualsIgnCase(comm, "yabai") &&
            !ffStrEqualsIgnCase(comm, "rectangle")
        ) continue;

        ffStrbufAppendS(pluginName, comm);
        pluginName->chars[0] = (char) toupper(pluginName->chars[0]);
        break;
    }

    return NULL;
}

const char* ffDetectWMVersion(const FFstrbuf* wmName, FFstrbuf* result, FF_MAYBE_UNUSED FFWMOptions* options)
{
    if (!wmName)
        return "No WM detected";

    if (ffStrbufEqualS(wmName, "WindowServer"))
    {
        NSError* error;
        NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL URLWithString:@"file:///System/Library/CoreServices/WindowManager.app/Contents/version.plist"]
                                           error:&error];
        ffStrbufInitS(result, ((NSString*) dict[@"CFBundleVersion"]).UTF8String);
    }

    return NULL;
}
