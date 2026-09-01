#include "wm.h"

#include "common/sysctl.h"
#include "common/mallocHelper.h"
#include "common/strutil.h"
#include "common/apple/version.h"

#include <ctype.h>
#include <libproc.h>
#import <Foundation/Foundation.h>

const char* ffDetectWMPlugin(FFstrbuf* pluginName) {
    int request[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL };
    size_t length;

    if (sysctl(request, ARRAY_SIZE(request), nullptr, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL, nullptr}) failed";
    }

    // The process table may change between the two sysctl calls; retry with a larger buffer.
    length += length / 8 + sizeof(struct kinfo_proc);
    FF_AUTO_FREE struct kinfo_proc* processes = malloc(length);

    if (sysctl(request, ARRAY_SIZE(request), processes, &length, nullptr, 0) != 0) {
        return "sysctl({CTL_KERN, KERN_PROC, KERN_PROC_ALL, processes}) failed";
    }

    uint32_t count = (uint32_t) (length / sizeof(struct kinfo_proc));

    for (size_t i = 0; i < count; i++) {
        const struct kinfo_proc* proc = &processes[i];
        if (proc->kp_eproc.e_ppid != 1) {
            continue;
        }

        const char* comm = proc->kp_proc.p_comm;

        if (
            !ffStrEqualsIgnCase(comm, "rectangle") && // 28.6k
            !ffStrEqualsIgnCase(comm, "yabai") &&     // 28.4k
            !ffStrEqualsIgnCase(comm, "aerospace") && // 19.6k
            !ffStrEqualsIgnCase(comm, "amethyst") &&  // 16k
            !ffStrEqualsIgnCase(comm, "glazewm") &&   // 11.6k

#if 0
            // Unmaintained
            !ffStrEqualsIgnCase(comm, "spectacle") && // 13.6k
            !ffStrEqualsIgnCase(comm, "chunkwm") && // repo deleted; was https://github.com/koekeishiya/chunkwm
            !ffStrEqualsIgnCase(comm, "kwm") && // repo deleted; was https://github.com/koekeishiya/kwm
#endif
            true)
            continue;

        if (instance.config.general.detectVersion) {
            char buf[PROC_PIDPATHINFO_MAXSIZE];
            int length = proc_pidpath(proc->kp_proc.p_pid, buf, ARRAY_SIZE(buf) - strlen("Info.plist"));
            if (length > 0) {
                buf[length] = '\0';
                FF_STRBUF_AUTO_DESTROY pluginVersion = ffStrbufCreate();
                if (ffGetAppNameAndVersion(buf, pluginName, &pluginVersion)) {
                    if (pluginName->length == 0) {
                        ffStrbufSetS(pluginName, comm);
                    }
                    if (pluginVersion.length > 0) {
                        ffStrbufAppendC(pluginName, ' ');
                        ffStrbufAppend(pluginName, &pluginVersion);
                    }
                    break;
                }
            }
        }

        ffStrbufAppendS(pluginName, comm);
        pluginName->chars[0] = (char) toupper(pluginName->chars[0]);
        break;
    }

    return nullptr;
}

const char* ffDetectWMVersion(const FFstrbuf* wmName, FFstrbuf* result, [[maybe_unused]] FFWMOptions* options) {
    if (!wmName) {
        return "No WM detected";
    }

    if (ffStrbufEqualS(wmName, "WindowServer")) {
        NSError* error;
        NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL fileURLWithPath:@"/System/Library/PrivateFrameworks/SkyLight.framework/Resources/version.plist" isDirectory:NO]
                                                                 error:&error];
        if (!dict) {
            dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL fileURLWithPath:@"/System/Library/Frameworks/ApplicationServices.framework/Frameworks/CoreGraphics.framework/Resources/version.plist" isDirectory:NO]
                                                       error:&error];
        }

        if (dict) {
            ffStrbufSetS(result, ((NSString*) dict[@"CFBundleShortVersionString"]).UTF8String);
        }
    }

    return nullptr;
}
