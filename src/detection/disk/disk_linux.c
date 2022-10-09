#include "disk.h"

#include <sys/statvfs.h>

void ffDetectDiskWithStatvfs(const char* folderPath, struct statvfs* fs, FFDiskResult* result);

const char* ffDiskAutodetectFolders(FFinstance* instance, FFlist* folders)
{
    FF_UNUSED(instance);

    struct statvfs fsRoot;
    int rootRet = statvfs(FASTFETCH_TARGET_DIR_ROOT"/", &fsRoot);
    if(rootRet != 0)
        return "statvfs(\"/\") failed";

    ffDetectDiskWithStatvfs(FASTFETCH_TARGET_DIR_ROOT"/", &fsRoot, (FFDiskResult*)ffListAdd(folders));

    struct statvfs fsHome;
    int homeRet = statvfs(FASTFETCH_TARGET_DIR_HOME, &fsHome);
    if(homeRet == 0 && (fsRoot.f_fsid != fsHome.f_fsid))
        ffDetectDiskWithStatvfs(FASTFETCH_TARGET_DIR_HOME, &fsHome, (FFDiskResult*)ffListAdd(folders));

    return NULL;
}
