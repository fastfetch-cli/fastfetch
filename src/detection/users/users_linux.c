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

const char* ffDetectUsers(FFlist* users)
{
    struct utmpx* n = NULL;
    setutxent();

next:
    while((n = getutxent()))
    {
        if(n->ut_type != USER_PROCESS)
            continue;

        FF_LIST_FOR_EACH(FFUserResult, user, *users)
        {
            if(ffStrbufEqualS(&user->name, n->ut_user))
                goto next;
        }

        FFUserResult* user = (FFUserResult*) ffListAdd(users);
        ffStrbufInitS(&user->name, n->ut_user);
        ffStrbufInitS(&user->hostName, n->ut_host);
        ffStrbufInitS(&user->sessionName, n->ut_line);
        #ifdef __linux__
        if(n->ut_addr_v6[0] || n->ut_addr_v6[1] || n->ut_addr_v6[2] || n->ut_addr_v6[3])
            ffStrbufInitF(&user->clientIp, "%u.%u.%u.%u", n->ut_addr_v6[0], n->ut_addr_v6[1], n->ut_addr_v6[2], n->ut_addr_v6[3]);
        else
        #endif
        ffStrbufInit(&user->clientIp);
        user->loginTime = (uint64_t) n->ut_tv.tv_sec * 1000 + (uint64_t) n->ut_tv.tv_usec / 1000;
    }

    return NULL;
}
