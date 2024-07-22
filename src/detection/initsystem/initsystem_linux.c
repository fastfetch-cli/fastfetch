#include "initsystem.h"
#include "common/processing.h"
#include <unistd.h>

const char* ffDetectInitSystem(FFInitSystemResult* result)
{
    const char* error = ffProcessGetBasicInfoLinux((int) result->pid, &result->name, NULL, NULL);
    if (error)
    {
        #ifdef __ANDROID__
        if (access("/system/bin/init", F_OK) == 0)
        {
            ffStrbufSetStatic(&result->exe, "/system/bin/init");
            ffStrbufSetStatic(&result->name, "init");
            return NULL;
        }
        #endif
        return error;
    }

    const char* _;
    // In linux /proc/1/exe is not readable
    ffProcessGetInfoLinux((int) result->pid, &result->name, &result->exe, &_, NULL);
    if (result->exe.chars[0] == '/')
    {
        // In some old system, /sbin/init is a symlink
        char buf[PATH_MAX];
        if (realpath(result->exe.chars, buf))
            ffStrbufSetS(&result->exe, buf);
    }

    if (instance.config.general.detectVersion)
    {
        if (ffStrbufEqualS(&result->name, "systemd"))
        {
            if (ffProcessAppendStdOut(&result->version, (char* const[]) {
                ffStrbufEndsWithS(&result->exe, "/systemd") ? result->exe.chars : "systemctl", // use exe path in case users have another systemd installed
                "--version",
                NULL,
            }) == NULL && result->version.length)
            {
                uint32_t iStart = ffStrbufFirstIndexC(&result->version, '(');
                if (iStart < result->version.length)
                {
                    uint32_t iEnd = ffStrbufNextIndexC(&result->version, iStart + 1, ')');
                    ffStrbufSubstrBefore(&result->version, iEnd);
                    ffStrbufSubstrAfter(&result->version, iStart);
                }
            }
        }
        else if (ffStrbufEqualS(&result->name, "launchd"))
        {
            if (ffProcessAppendStdOut(&result->version, (char* const[]) {
                "/bin/launchctl",
                "version",
                NULL,
            }) == NULL && result->version.length)
            {
                uint32_t iStart = ffStrbufFirstIndexS(&result->version, "Version ");
                if (iStart < result->version.length)
                {
                    iStart += (uint32_t) strlen("Version");
                    uint32_t iEnd = ffStrbufNextIndexC(&result->version, iStart + 1, ':');
                    ffStrbufSubstrBefore(&result->version, iEnd);
                    ffStrbufSubstrAfter(&result->version, iStart);
                }
            }
        }
    }

    return NULL;
}
