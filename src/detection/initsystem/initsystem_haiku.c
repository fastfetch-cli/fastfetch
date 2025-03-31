#include "initsystem.h"
#include "util/stringUtils.h"
#include "util/haiku/version.h"
#include "common/io/io.h"

#include <OS.h>
#include <unistd.h>

const char* ffDetectInitSystem(FFInitSystemResult* result)
{
    // Since it runs first, registrar does not know about it,
    // so we can't query be_roster for it.
    const char* path = "/boot/system/servers/launch_daemon";
    if (!ffPathExists(path, FF_PATHTYPE_FILE))
        return "launch_daemon is not found";

    ffStrbufSetStatic(&result->exe, path);
    ffStrbufSetStatic(&result->name, "launch_daemon");
    result->pid = 0;

    team_info teamInfo;
    int32 cookie = 0;
    while (get_next_team_info(&cookie, &teamInfo) == B_OK)
    {
        if (ffStrEquals(teamInfo.args, path))
        {
            result->pid = (uint32_t) teamInfo.team;
            break;
        }
    }

    if (instance.config.general.detectVersion)
        ffGetFileVersion(path, &result->version);

    return NULL;
}
