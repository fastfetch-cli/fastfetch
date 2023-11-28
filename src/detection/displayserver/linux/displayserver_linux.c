#include "displayserver_linux.h"
#include "common/io/io.h"
#include "util/edidHelper.h"
#include "util/stringUtils.h"

#include <dirent.h>
#include <fcntl.h>

// 2 * (uint32_t string max length is 10 bytes) + 'x' + NUL
#define MODE_FILE_CONTENT_LENGTH 22

static void parseDRM(FFDisplayServerResult* result)
{
    const char* drmDirPath = "/sys/class/drm/";

    DIR* dirp = opendir(drmDirPath);
    if(dirp == NULL)
        return;

    FF_STRBUF_AUTO_DESTROY drmDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&drmDir, drmDirPath);

    uint32_t drmDirLength = drmDir.length;

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(ffStrEquals(entry->d_name, ".") || ffStrEquals(entry->d_name, ".."))
            continue;

        ffStrbufAppendS(&drmDir, entry->d_name);
        ffStrbufAppendS(&drmDir, "/modes");

        int FF_AUTO_CLOSE_FD modeFileFd = open(drmDir.chars, O_RDONLY);
        if(modeFileFd < 0)
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            continue;
        }

        uint32_t width = 0, height = 0;

        char buf[MODE_FILE_CONTENT_LENGTH] = {0x00};
        if (read(modeFileFd, buf, sizeof(char) * (MODE_FILE_CONTENT_LENGTH - 1)) > 0) {
            char *sep = strchr(buf, 'x');
            if (sep != NULL) {
                *sep = '\0';
                width = (uint32_t)atoi(buf);
                height = (uint32_t)atoi(sep + 1);
            }
        }

        if(width > 0 && height > 0)
        {
            ffStrbufSubstrBefore(&drmDir, drmDirLength);
            ffStrbufAppendS(&drmDir, entry->d_name);
            ffStrbufAppendS(&drmDir, "/edid");

            FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
            uint8_t edidData[128];
            if(ffReadFileData(drmDir.chars, sizeof(edidData), edidData) == sizeof(edidData))
                ffEdidGetName(edidData, &name);
            else
            {
                const char* plainName = entry->d_name;
                if (ffStrStartsWith(plainName, "card"))
                {
                    const char* tmp = strchr(plainName + strlen("card"), '-');
                    if (tmp) plainName = tmp + 1;
                }
                ffStrbufAppendS(&name, plainName);
            }

            ffdsAppendDisplay(
                result,
                width, height,
                0,
                0, 0,
                0,
                &name,
                FF_DISPLAY_TYPE_UNKNOWN,
                false,
                0
            );
        }

        ffStrbufSubstrBefore(&drmDir, drmDirLength);
    }

    closedir(dirp);
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds)
{
    ffStrbufInit(&ds->wmProcessName);
    ffStrbufInit(&ds->wmPrettyName);
    ffStrbufInit(&ds->wmProtocolName);
    ffStrbufInit(&ds->deProcessName);
    ffStrbufInit(&ds->dePrettyName);
    ffListInitA(&ds->displays, sizeof(FFDisplayResult), 4);

    if (!instance.config.general.dsForceDrm)
    {
        //We try wayland as our preferred display server, as it supports the most features.
        //This method can't detect the name of our WM / DE
        ffdsConnectWayland(ds);

        //Try the x11 libs, from most feature rich to least.
        //We use the display list to detect if a connection is needed.
        //They respect wmProtocolName, and only detect display if it is set.

        if(ds->displays.length == 0)
            ffdsConnectXcbRandr(ds);

        if(ds->displays.length == 0)
            ffdsConnectXrandr(ds);

        if(ds->displays.length == 0)
            ffdsConnectXcb(ds);

        if(ds->displays.length == 0)
            ffdsConnectXlib(ds);
    }

    //This display detection method is display server independent.
    //Use it if all connections failed
    if(ds->displays.length == 0)
        parseDRM(ds);

    //This fills in missing information about WM / DE by using env vars and iterating processes
    ffdsDetectWMDE(ds);
}
