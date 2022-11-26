#include "users.h"
#include "util/windows/unicode.h"

#include <wtsapi32.h>

//at the time of writing, <wtsapi32.h> of MinGW doesn't have the definition of WTSEnumerateSessionsExW
typedef struct _WTS_SESSION_INFO_1W {
    DWORD ExecEnvId;
    WTS_CONNECTSTATE_CLASS State;
    DWORD SessionId;
    LPWSTR pSessionName;
    LPWSTR pHostName;
    LPWSTR pUserName;
    LPWSTR pDomainName;
    LPWSTR pFarmName;
} WTS_SESSION_INFO_1W, * PWTS_SESSION_INFO_1W;

BOOL
WINAPI
WTSEnumerateSessionsExW(
    HANDLE hServer,
    DWORD* pLevel,
    DWORD Filter,
    PWTS_SESSION_INFO_1W* ppSessionInfo,
    DWORD* pCount);

void ffDetectUsers(FFlist* users, FFstrbuf* error)
{
    WTS_SESSION_INFO_1W* sessionInfo;
    DWORD sessionCount;
    DWORD level = 1;

    if(!WTSEnumerateSessionsExW(WTS_CURRENT_SERVER_HANDLE, &level, 0, &sessionInfo, &sessionCount))
    {
        ffStrbufAppendS(error, "WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE) failed");
        return;
    }

    for (DWORD i = 0; i < sessionCount; i++)
    {
        WTS_SESSION_INFO_1W* session = &sessionInfo[i];
        if(session->State != WTSActive)
            continue;

        FF_STRBUF_AUTO_DESTROY domainName = ffStrbufCreateWS(session->pDomainName);
        FF_STRBUF_AUTO_DESTROY userName = ffStrbufCreateWS(session->pUserName);

        ffStrbufInitF((FFstrbuf*)ffListAdd(users), "%s\\%s", domainName.chars, userName.chars);
    }

    WTSFreeMemory(sessionInfo);

    if(users->length == 0)
        ffStrbufAppendS(error, "Unable to detect users");
}
