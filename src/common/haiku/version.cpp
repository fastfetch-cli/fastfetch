extern "C" {
#include "version.h"
}

#include <File.h>
#include <AppFileInfo.h>

bool ffGetFileVersion(const char* filePath, FFstrbuf* version)
{
    BFile f(filePath, B_READ_ONLY);
    if (f.InitCheck() != B_OK)
        return false;

    BAppFileInfo fileInfo(&f);
    if (f.InitCheck() != B_OK)
        return false;

    version_info info;
    if (fileInfo.GetVersionInfo(&info, B_SYSTEM_VERSION_KIND) != B_OK)
        return false;

    ffStrbufSetF(version, "%d.%d.%d", (int)info.major, (int)info.middle, (int)info.minor);
    return true;
}
