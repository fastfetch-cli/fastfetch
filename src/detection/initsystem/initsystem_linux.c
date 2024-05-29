#include "initsystem.h"
#include "common/processing.h"

const char* ffDetectInitSystem(FFInitSystemResult* result)
{
    const char* error = ffProcessGetBasicInfoLinux((int) result->pid, &result->name, NULL, NULL);
    if (error) return error;

    const char* exeName;

    ffProcessGetInfoLinux((int) result->pid, &result->name, &result->exe, &exeName, NULL);

    if (ffStrbufEqualS(&result->name, "systemd"))
    {
        if (ffProcessAppendStdOut(&result->version, (char* const[]) {
            "systemctl",
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
            else
            {
                iStart = ffStrbufFirstIndexC(&result->version, ' ');
                if (iStart < result->version.length)
                {
                    uint32_t iEnd = ffStrbufNextIndexC(&result->version, iStart + 1, ' ');
                    ffStrbufSubstrBefore(&result->version, iEnd);
                    ffStrbufSubstrAfter(&result->version, iStart);
                }
            }
        }
    }
    else if (ffStrbufEqualS(&result->name, "launchd"))
    {
        if (ffProcessAppendStdOut(&result->version, (char* const[]) {
            "launchctl",
            "version",
            NULL,
        }) == NULL && result->version.length)
        {
            uint32_t iStart = ffStrbufFirstIndexS(&result->version, "Version ");
            if (iStart < result->version.length)
            {
                iStart += strlen("Version");
                uint32_t iEnd = ffStrbufNextIndexC(&result->version, iStart + 1, ':');
                ffStrbufSubstrBefore(&result->version, iEnd);
                ffStrbufSubstrAfter(&result->version, iStart);
            }
        }
    }

    return NULL;
}
