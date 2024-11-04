#include "fastfetch.h"
#include "users.h"
#include "common/io/io.h"

#include <utmp.h>

const char* ffDetectUsers(FF_MAYBE_UNUSED FFUsersOptions* options, FFlist* users)
{
    FF_AUTO_CLOSE_FILE FILE* fp = fopen(_PATH_UTMP, "r");
    if (!fp) return "fopen(_PATH_UTMP, r) failed";

    struct utmp n;
next:
    while (fread(&n, sizeof(n), 1, fp) == 1)
    {
        if (!n.ut_name[0]) continue;

        if (options->myselfOnly && !ffStrbufEqualS(&instance.state.platform.userName, n.ut_name))
            continue;

        FF_LIST_FOR_EACH(FFUserResult, user, *users)
        {
            if (ffStrbufEqualS(&user->name, n.ut_name))
                goto next;
        }

        FFUserResult* user = (FFUserResult*) ffListAdd(users);
        ffStrbufInitS(&user->name, n.ut_name);
        ffStrbufInitS(&user->hostName, n.ut_host);
        ffStrbufInitS(&user->sessionName, n.ut_line);
        ffStrbufInit(&user->clientIp);
        user->loginTime = (uint64_t) n.ut_time * 1000;
    }
    
    return NULL;
}
