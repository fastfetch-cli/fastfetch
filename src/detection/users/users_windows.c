#include "users.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <wtsapi32.h>

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

    WTSFreeMemoryExW(WTSTypeSessionInfoLevel1, sessionInfo, 1);

    if(users->length == 0)
        ffStrbufAppendS(error, "Unable to detect users");
}
