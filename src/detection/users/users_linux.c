#include "fastfetch.h"
#include "users.h"

#if FF_HAVE_UTMPX
    #include <utmpx.h>
#else
    //for Android compatibility
    #include <utmp.h>
    #define utmpx utmp
    #define setutxent setutent
    #define getutxent getutent
#endif
#ifdef __linux__
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

const char* ffDetectUsers(FFUsersOptions* options, FFlist* users)
{
    struct utmpx* n = NULL;
    setutxent();

next:
    while ((n = getutxent()))
    {
        if (n->ut_type != USER_PROCESS)
            continue;

        if (options->myselfOnly && !ffStrbufEqualS(&instance.state.platform.userName, n->ut_user))
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
        // https://www.linuxquestions.org/questions/programming-9/get-the-ip-addr-out-from-an-int32_t-value-287687/#post1458622
        ffStrbufInitS(&user->clientIp, inet_ntoa((struct in_addr) { .s_addr = (in_addr_t) n->ut_addr_v6[0] }));
        #else
        ffStrbufInit(&user->clientIp);
        #endif
        user->loginTime = (uint64_t) n->ut_tv.tv_sec * 1000 + (uint64_t) n->ut_tv.tv_usec / 1000;
    }

    return NULL;
}
