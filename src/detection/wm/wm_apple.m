#include "wm.h"

#include "common/sysctl.h"
#include "common/mallocHelper.h"
#include "common/stringUtils.h"

#include <ctype.h>
#include <libproc.h>
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
        const struct kinfo_proc* proc = &processes[i];
        if (proc->kp_eproc.e_ppid != 1) continue;

        const char* comm = proc->kp_proc.p_comm;

        if(
            !ffStrEqualsIgnCase(comm, "spectacle") &&
            !ffStrEqualsIgnCase(comm, "amethyst") &&
            !ffStrEqualsIgnCase(comm, "kwm") &&
            !ffStrEqualsIgnCase(comm, "chunkwm") &&
            !ffStrEqualsIgnCase(comm, "yabai") &&
            !ffStrEqualsIgnCase(comm, "aerospace") &&
            !ffStrEqualsIgnCase(comm, "rectangle")
        ) continue;

        if (instance.config.general.detectVersion)
        {
            char buf[PROC_PIDPATHINFO_MAXSIZE];
            int length = proc_pidpath(proc->kp_proc.p_pid, buf, ARRAY_SIZE(buf) - strlen("Info.plist"));
            if (length > 0)
            {
                char* lastSlash = strrchr(buf, '/');
                if (lastSlash)
                {
                    *lastSlash = '\0';
                    if (ffStrEndsWith(buf, ".app/Contents/MacOS"))
                    {
                        lastSlash -= strlen("MacOS");
                        strcpy(lastSlash, "Info.plist"); // X.app/Contents/Info.plist
                        NSError* error;
                        NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL fileURLWithPath:@(buf)]
                                                        error:&error];
                        if (dict)
                        {
                            NSString* name = dict[@"CFBundleDisplayName"] ?: dict[@"CFBundleName"];
                            ffStrbufSetS(pluginName, name.UTF8String ?: comm);

                            NSString* version = dict[@"CFBundleShortVersionString"];
                            if (version)
                            {
                                ffStrbufAppendC(pluginName, ' ');
                                ffStrbufAppendS(pluginName, version.UTF8String);
                            }

                            break;
                        }
                    }
                }
            }
        }

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
        NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL fileURLWithPath:@"/System/Library/PrivateFrameworks/SkyLight.framework/Resources/version.plist" isDirectory:NO]
                                           error:&error];
        if (!dict)
        {
            dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL fileURLWithPath:@"/System/Library/Frameworks/ApplicationServices.framework/Frameworks/CoreGraphics.framework/Resources/version.plist" isDirectory:NO]
                                           error:&error];
        }

        if (dict)
            ffStrbufSetS(result, ((NSString*) dict[@"CFBundleShortVersionString"]).UTF8String);
    }

    return NULL;
}
