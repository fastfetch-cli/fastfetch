extern "C" {
#include "initsystem.h"
#include "common/processing.h"
#include "util/binary.h"
#include "util/stringUtils.h"
};

#include <AppFileInfo.h>
#include <File.h>
#include <libgen.h>
#include <OS.h>
#include <Path.h>
#include <unistd.h>

const char* ffDetectInitSystem(FFInitSystemResult* result)
{
    team_info teamInfo;
    int32 cookie = 0;

    // Since it runs first, registrar does not know about it,
    // so we can't query be_roster for it.
    BPath path("/boot/system/servers/launch_daemon");
    if (path.InitCheck() != B_OK)
        return NULL;

    ffStrbufSetS(&result->exe, path.Path());
    ffStrbufSetStatic(&result->name, "launch_daemon");

    while (get_next_team_info(&cookie, &teamInfo) == B_OK)
    {
        if (strcmp(teamInfo.args, path.Path()) == 0)
        {
            result->pid = (int)teamInfo.team;
            break;
        }
    }

    BFile f(path.Path(), B_READ_ONLY);
    if (f.InitCheck() != B_OK)
        return NULL;
    BAppFileInfo fileInfo(&f);
    if (f.InitCheck() != B_OK)
        return NULL;

    version_info version;
    if (fileInfo.GetVersionInfo(&version, B_SYSTEM_VERSION_KIND) != B_OK)
        return NULL;

    ffStrbufSetF(&result->version, "%d.%d.%d", (int)version.major, (int)version.middle, (int)version.minor);

    return NULL;
}
