#include "initsystem.h"
#include "common/processing.h"
#include "util/binary.h"
#include "util/stringUtils.h"

#include <libgen.h>
#include <unistd.h>

FF_MAYBE_UNUSED static bool extractSystemdVersion(const char* str, uint32_t len, void* userdata)
{
    if (!ffStrStartsWith(str, "systemd ")) return true;
    const char* pstart = str + strlen("systemd ");
    const char* pend = memmem(pstart, len - strlen("systemd "), " running in ", strlen(" running in "));
    if (!pend) return true;
    ffStrbufSetNS((FFstrbuf*) userdata, (uint32_t) (pend - pstart), pstart);
    return false;
}

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
        {
            ffStrbufSetS(&result->exe, buf);
            ffStrbufSetS(&result->name, basename(result->exe.chars));
        }
    }

    if (instance.config.general.detectVersion)
    {
        #if __linux__ && !__ANDROID__
        if (ffStrbufEqualS(&result->name, "systemd"))
        {
            ffBinaryExtractStrings(result->exe.chars, extractSystemdVersion, &result->version, (uint32_t) strlen("systemd 0.0 running in x"));
            if (result->version.length == 0)
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
        }
        else if (ffStrbufEqualS(&result->name, "dinit"))
        {
            if (ffProcessAppendStdOut(&result->version, (char* const[]) {
                ffStrbufEndsWithS(&result->exe, "/dinit") ? result->exe.chars : "dinit",
                "--version",
                NULL,
            }) == NULL && result->version.length)
            {
                // Dinit version 0.18.0.
                ffStrbufSubstrBeforeFirstC(&result->version, '\n');
                ffStrbufTrimRight(&result->version, '.');
                ffStrbufSubstrAfterLastC(&result->version, ' ');
            }
        }
        #elif __APPLE__
        if (ffStrbufEqualS(&result->name, "launchd"))
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
        #endif
    }

    return NULL;
}
