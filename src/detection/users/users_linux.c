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
#if __linux__ || __GNU__
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
        ffStrbufInit(&user->clientIp);
        #if __linux__ || __GNU__
        bool isIpv6 = false;
        for (int i = 1; i < 4; ++i) {
            if (n->ut_addr_v6[i] != 0) {
                isIpv6 = true;
                break;
            }
        }

        if (isIpv6) {
            char ipv6_str[INET6_ADDRSTRLEN];
            if (inet_ntop(AF_INET6, n->ut_addr_v6, ipv6_str, INET6_ADDRSTRLEN) != NULL) {
                ffStrbufSetS(&user->clientIp, ipv6_str);
            }
        } else if (n->ut_addr_v6[0] != 0) {
            char ipv4_str[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, n->ut_addr_v6, ipv4_str, INET_ADDRSTRLEN) != NULL) {
                ffStrbufSetS(&user->clientIp, ipv4_str);
            }
        }
        #endif
        user->loginTime = (uint64_t) n->ut_tv.tv_sec * 1000 + (uint64_t) n->ut_tv.tv_usec / 1000;
    }

    return NULL;
}
