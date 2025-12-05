#include "common/io/io.h"
#include "common/properties.h"
#include "fastfetch.h"
#include "users.h"

#include <unistd.h>

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

#if __linux__
bool detectUserBySystemd(const FFstrbuf* pathUsers, FFlist* users)
{
    FF_STRBUF_AUTO_DESTROY state = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY userName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY loginTime = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY sessions = ffStrbufCreate();

    // WARNING: This is private data. Do not parse
    if (!ffParsePropFileValues(pathUsers->chars, 4, (FFpropquery[]) {
        {"NAME=", &userName},
        {"STATE=", &state},
        {"REALTIME=", &loginTime},
        {"ONLINE_SESSIONS=", &sessions},
    }) || !ffStrbufEqualS(&state, "active"))
        return false;

    FFUserResult* user = FF_LIST_ADD(FFUserResult, *users);
    ffStrbufInitMove(&user->name, &userName);
    ffStrbufInit(&user->hostName);
    ffStrbufInit(&user->sessionName);
    ffStrbufInit(&user->clientIp);
    ffStrbufSubstrBefore(&loginTime, loginTime.length - 3); // converts us to ms
    user->loginTime = ffStrbufToUInt(&loginTime, 0);

    FF_STRBUF_AUTO_DESTROY pathSessions = ffStrbufCreateS("/run/systemd/sessions/");
    const uint32_t pathSessionsBaseLen = pathSessions.length;

    FF_STRBUF_AUTO_DESTROY tty = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY remoteHost = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY service = ffStrbufCreate();

    char* token = NULL;
    size_t n = 0;
    while (ffStrbufGetdelim(&token, &n, ' ', &sessions))
    {
        ffStrbufSubstrBefore(&pathSessions, pathSessionsBaseLen);
        ffStrbufAppendS(&pathSessions, token);

        ffStrbufClear(&remoteHost);
        ffStrbufClear(&service);
        ffStrbufClear(&tty);
        ffStrbufClear(&loginTime);

        // WARNING: This is private data. Do not parse
        if (ffParsePropFileValues(pathSessions.chars, 4, (FFpropquery[]) {
            {"REMOTE_HOST=", &remoteHost},
            {"TTY=", &tty},
            {"SERVICE=", &service},
            {"REALTIME=", &loginTime},
        }) && !ffStrbufEqualS(&service, "systemd-user"))
        {
            if (remoteHost.length)
            {
                ffStrbufTrimRight(&remoteHost, ']');
                ffStrbufTrimLeft(&remoteHost, '[');
                ffStrbufInitMove(&user->hostName, &remoteHost);
            }
            else
                ffStrbufSetStatic(&user->hostName, "localhost");
            ffStrbufInitMove(&user->sessionName, tty.length ? &tty : &service);
            if (loginTime.length)
            {
                ffStrbufSubstrBefore(&loginTime, loginTime.length - 3); // converts us to ms
                user->loginTime = ffStrbufToUInt(&loginTime, 0);
            }
            break;
        }
    }

    return true;
}

const char* detectBySystemd(FFUsersOptions* options, FFlist* users)
{
    // For some reason, debian/ubuntu no longer updates `/var/run/utmp` (#2064)
    // Query systemd instead
    FF_STRBUF_AUTO_DESTROY pathUsers = ffStrbufCreateS("/run/systemd/users/");

    if (options->myselfOnly)
    {
        ffStrbufAppendUInt(&pathUsers, getuid());
        detectUserBySystemd(&pathUsers, users);
    }
    else
    {
        const uint32_t pathUsersBaseLen = pathUsers.length;
        FF_AUTO_CLOSE_DIR DIR* dirp = opendir(pathUsers.chars);
        if (!dirp) return "opendir(\"/run/systemd/users/\") failed";

        struct dirent* entry;
        while ((entry = readdir(dirp)))
        {
            if (entry->d_type != DT_REG) continue;

            ffStrbufAppendS(&pathUsers, entry->d_name);
            detectUserBySystemd(&pathUsers, users);
            ffStrbufSubstrBefore(&pathUsers, pathUsersBaseLen);
        }
    }
    return NULL;
}
#endif

#if __linux__ || __GNU__
static void fillUtmpIpAddr(FFUserResult* user, struct utmpx* n)
{
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
}
#else
static void fillUtmpIpAddr(FF_MAYBE_UNUSED FFUserResult* user, FF_MAYBE_UNUSED struct utmpx* n)
{
}
#endif

const char* detectByUtmp(FFUsersOptions* options, FFlist* users)
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
            {
                uint64_t newLoginTime = (uint64_t) n->ut_tv.tv_sec * 1000 + (uint64_t) n->ut_tv.tv_usec / 1000;
                if (newLoginTime > user->loginTime)
                {
                    ffStrbufSetS(&user->hostName, n->ut_host);
                    ffStrbufSetS(&user->sessionName, n->ut_line);
                    fillUtmpIpAddr(user, n);
                    user->loginTime = newLoginTime;
                }
                goto next;
            }
        }

        FFUserResult* user = FF_LIST_ADD(FFUserResult, *users);
        ffStrbufInitS(&user->name, n->ut_user);
        ffStrbufInitS(&user->hostName, n->ut_host);
        ffStrbufInitS(&user->sessionName, n->ut_line);
        ffStrbufInit(&user->clientIp);
        fillUtmpIpAddr(user, n);
        user->loginTime = (uint64_t) n->ut_tv.tv_sec * 1000 + (uint64_t) n->ut_tv.tv_usec / 1000;
    }

    endutxent();

    return NULL;
}

const char* ffDetectUsers(FFUsersOptions* options, FFlist* users)
{
    const char* err = detectByUtmp(options, users);
    if (err) return err;

    #if __linux__
    if (users->length == 0) detectBySystemd(options, users);
    #endif

    return NULL;
}
