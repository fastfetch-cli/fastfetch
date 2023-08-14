#include "fastfetch.h"
#include "users.h"

#if __has_include(<utmpx.h>)
    #include <utmpx.h>
#else
    //for Android compatibility
    #include <utmp.h>
    #define utmpx utmp
    #define setutxent setutent
    #define getutxent getutent
#endif

void ffDetectUsers(FFlist* users, FFstrbuf* error)
{
    struct utmpx* n = NULL;
    setutxent();

next:
    while((n = getutxent()))
    {
        if(n->ut_type != USER_PROCESS)
            continue;

        FF_LIST_FOR_EACH(FFstrbuf, user, *users)
        {
            if(ffStrbufEqualS(user, n->ut_user))
                goto next;
        }

        ffStrbufInitS((FFstrbuf*)ffListAdd(users), n->ut_user);
    }

    if(users->length == 0)
        ffStrbufAppendS(error, "Unable to detect users");
}
