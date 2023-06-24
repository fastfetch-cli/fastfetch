#include "lm.h"
#include "common/properties.h"
#include "common/dbus.h"
#include "detection/displayserver/displayserver.h"

#include <unistd.h>

#define FF_SYSTEMD_SESSIONS_PATH "/var/run/systemd/sessions/"
#define FF_SYSTEMD_USERS_PATH "/run/systemd/users/"

const char* ffDetectLM(FFLMResult* result)
{
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();

    FF_STRBUF_AUTO_DESTROY sessionId = ffStrbufCreateS(getenv("XDG_SESSION_ID"));
    if (sessionId.length == 0)
    {
        // On some incorrectly configured systems, $XDG_SESSION_ID is not set. Try finding it ourself
        // WARNING: This is private data. Do not parse
        ffStrbufAppendF(&path, "/run/systemd/users/%d", getuid());

        // This is actually buggy, and assumes current user is using DE
        // `sd_pid_get_session` can be a better option, but we need to find a pid to use
        if (!ffParsePropFileValues(path.chars, 1, (FFpropquery[]) {
            {"DISPLAY=", &sessionId},
        }))
            return "Failed to get $XDG_SESSION_ID";
        ffStrbufClear(&path);
    }

    ffStrbufAppendS(&path, "/var/run/systemd/sessions/");
    ffStrbufAppend(&path, &sessionId);

    // WARNING: This is private data. Do not parse
    if (!ffParsePropFileValues(path.chars, 2, (FFpropquery[]) {
        {"SERVICE=", &result->service},
        {"TYPE=", &result->type},
    }))
        return "Failed to parse /run/systemd/sessions/$XDG_SESSION_ID";

    // Correct char cases
    if (ffStrbufIgnCaseEqualS(&result->type, FF_WM_PROTOCOL_WAYLAND))
        ffStrbufSetS(&result->type, FF_WM_PROTOCOL_WAYLAND);
    else if (ffStrbufIgnCaseEqualS(&result->type, FF_WM_PROTOCOL_X11))
        ffStrbufSetS(&result->type, FF_WM_PROTOCOL_X11);
    else if (ffStrbufIgnCaseEqualS(&result->type, FF_WM_PROTOCOL_TTY))
        ffStrbufSetS(&result->type, FF_WM_PROTOCOL_TTY);

    return NULL;
}
