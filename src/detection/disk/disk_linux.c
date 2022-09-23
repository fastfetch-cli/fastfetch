#include "disk.h"

#include <sys/statvfs.h>

const char* ffDiskAutodetectFolders(FFinstance* instance, FFlist* folders)
{
    FF_UNUSED(instance);

    struct statvfs fsRoot;
    int rootRet = statvfs(FASTFETCH_TARGET_DIR_ROOT"/", &fsRoot);
    if(rootRet != 0)
        return "statvfs(\"/\") failed";

    ffStrbufInitS((FFstrbuf *)ffListAdd(folders), FASTFETCH_TARGET_DIR_ROOT"/");

    struct statvfs fsHome;
    int homeRet = statvfs(FASTFETCH_TARGET_DIR_HOME, &fsHome);
    if(homeRet == 0 && (fsRoot.f_fsid != fsHome.f_fsid))
        ffStrbufInitS((FFstrbuf *)ffListAdd(folders), FASTFETCH_TARGET_DIR_HOME);

    return NULL;
}
