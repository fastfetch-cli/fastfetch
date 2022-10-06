#include "fastfetch.h"
#include "disk.h"

#include <sys/statvfs.h>

void ffDetectDiskWithStatvfs(const char* folderPath, struct statvfs* fs, FFDiskResult* result)
{
    ffStrbufInitS(&result->path, folderPath);
    result->used = result->total = 0;
    ffStrbufInit(&result->error);

    struct statvfs newFs;

    if(fs == NULL)
    {
        fs = &newFs;
        int ret = statvfs(folderPath, fs);
        if(ret != 0)
        {
            ffStrbufAppendF(&result->error, "statvfs(\"%s\", &fs) != 0 (%i)", folderPath, ret);
            return;
        }
    }

    result->total = fs->f_blocks * fs->f_frsize;

    if(result->total == 0)
    {
        ffStrbufAppendF(&result->error, "statvfs for %s returned size 0", folderPath);
        return;
    }

    result->used = result->total - (fs->f_bavail * fs->f_frsize);
    result->files = (uint32_t) (fs->f_files - fs->f_ffree);
    result->removable = false; //To be set at other place
}

bool ffDiskDetectDiskFolders(FFinstance* instance, FFlist* folders)
{
    ffStrbufTrim(&instance->config.diskFolders, ':');
    if(instance->config.diskFolders.length == 0)
        return false;

    uint32_t startIndex = 0;
    while(startIndex < instance->config.diskFolders.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&instance->config.diskFolders, startIndex, ':');
        instance->config.diskFolders.chars[colonIndex] = '\0';

        ffDetectDiskWithStatvfs(instance->config.diskFolders.chars + startIndex, NULL, (FFDiskResult*)ffListAdd(folders));

        startIndex = colonIndex + 1;
    }

    return true;
}
