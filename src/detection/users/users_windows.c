#include "users.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <wtsapi32.h>

static inline uint64_t to_ms(uint64_t ret)
{
    ret -= 116444736000000000ull;
    return ret / 10000ull;
}

const char* ffDetectUsers(FFUsersOptions* options, FFlist* users)
{
    WTS_SESSION_INFO_1W* sessionInfo;
    DWORD sessionCount;
    DWORD level = 1;

    if (!WTSEnumerateSessionsExW(WTS_CURRENT_SERVER_HANDLE, &level, 0, &sessionInfo, &sessionCount))
        return "WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE) failed";

    for (DWORD i = 0; i < sessionCount; i++)
    {
        WTS_SESSION_INFO_1W* session = &sessionInfo[i];
        if (session->State != WTSActive)
            continue;

        FF_STRBUF_AUTO_DESTROY userName = ffStrbufCreateWS(session->pUserName);

        if (options->myselfOnly && !ffStrbufEqual(&instance.state.platform.userName, &userName))
            continue;

        FFUserResult* user = (FFUserResult*) ffListAdd(users);
        ffStrbufInitMove(&user->name, &userName);
        ffStrbufInitWS(&user->hostName, session->pHostName);
        ffStrbufInitWS(&user->sessionName, session->pSessionName);
        ffStrbufInit(&user->clientIp);
        user->loginTime = 0;

        DWORD bytes = 0;
        PWTS_CLIENT_ADDRESS address = NULL;
        if (WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, session->SessionId, WTSClientAddress, (LPWSTR *) &address, &bytes))
        {
            if (address->AddressFamily == 2 /*AF_INET*/)
                ffStrbufSetF(&user->clientIp, "%u.%u.%u.%u", address->Address[2], address->Address[3], address->Address[4], address->Address[5]);
            WTSFreeMemory(address);
        }

        bytes = 0;
        PWTSINFOW wtsInfo = NULL;
        if (WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, session->SessionId, WTSSessionInfo, (LPWSTR *) &wtsInfo, &bytes))
        {
            user->loginTime = to_ms(*(uint64_t*) &wtsInfo->LogonTime);
            WTSFreeMemory(wtsInfo);
        }
    }

    WTSFreeMemoryExW(WTSTypeSessionInfoLevel1, sessionInfo, 1);

    return NULL;
}
