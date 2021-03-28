#include "fastfetch.h"

#include <sys/statvfs.h>

static void printStatvfs(FFinstance* instance, const char* key, struct statvfs* fs)
{
    const uint32_t GB = 1024 * 1024 * 1024;

    uint32_t total     = (fs->f_blocks * fs->f_frsize) / GB;
    uint32_t available = (fs->f_bfree  * fs->f_frsize) / GB;
    uint32_t used      = total - available;
    uint8_t percentage = (used / (double) total) * 100.0;

    uint32_t files = fs->f_files - fs->f_ffree;

    if(instance->config.diskFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, key);
        printf("%uGB / %uGB (%u%%)\n", used, total, percentage);
    }
    else
    {
        ffPrintFormatString(instance, key, &instance->config.diskFormat, 4,
            (FFformatarg){FF_FORMAT_ARG_TYPE_UINT, &used},
            (FFformatarg){FF_FORMAT_ARG_TYPE_UINT, &total},
            (FFformatarg){FF_FORMAT_ARG_TYPE_UINT, &files},
            (FFformatarg){FF_FORMAT_ARG_TYPE_UINT8, &percentage}
        );
    }
}

static void printFolder(FFinstance* instance, const char* folderPath)
{
    FF_STRBUF_CREATE(key);
    ffStrbufAppendS(&key, "Disk (");
    ffStrbufAppendS(&key, folderPath);
    ffStrbufAppendC(&key, ')');

    struct statvfs fs;
    int ret = statvfs(folderPath, &fs);
    if(ret != 0 && instance->config.diskFormat.length == 0)
    {
        ffPrintError(instance, key.chars, "statvfs(\"%s\", &fs) != 0", folderPath);
        ffStrbufDestroy(&key);
        return;
    }

    printStatvfs(instance, key.chars, &fs);

    ffStrbufDestroy(&key);
}

void ffPrintDisk(FFinstance* instance)
{
    if(instance->config.diskFolders.length == 0)
    {
        struct statvfs fsRoot;
        int rootRet = statvfs("/", &fsRoot);

        struct statvfs fsHome;
        int homeRet = statvfs("/home", &fsHome);

        if(rootRet != 0 && homeRet != 0)
        {
            ffPrintError(instance, "Disk", "statvfs failed for both / and /home");
            return;
        }

        if(rootRet == 0)
            printStatvfs(instance, "Disk (/)", &fsRoot);

        if(homeRet == 0 && (rootRet != 0 || fsRoot.f_fsid != fsHome.f_fsid))
            printStatvfs(instance, "Disk (/home)", &fsHome);
    }
    else
    {
        uint32_t lastIndex = 0;
        while (lastIndex < instance->config.diskFolders.length)
        {
            uint32_t colonIndex = ffStrbufFirstIndexAfterC(&instance->config.diskFolders, lastIndex, ':');
            instance->config.diskFolders.chars[colonIndex] = '\0';

            printFolder(instance, instance->config.diskFolders.chars + lastIndex);

            lastIndex = colonIndex + 1;
        }
    }
}
