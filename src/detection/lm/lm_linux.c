#include "lm.h"
#include "common/properties.h"

#define FF_SYSTEMD_SESSIONS_PATH "/var/run/systemd/sessions/"

const char* ffDetectLM(FFLMResult* result)
{
    const char* sessionId = getenv("XDG_SESSION_ID");
    if (!sessionId) return "$XDG_SESSION_ID is not set";

    char path[64] = FF_SYSTEMD_SESSIONS_PATH;
    strncpy(path + strlen(FF_SYSTEMD_SESSIONS_PATH), sessionId, sizeof(path) - strlen(FF_SYSTEMD_SESSIONS_PATH) - 1);

    // The file reads `This is private data. Do not parse`.
    // What does it mean? But let's ignore it for now.
    if (!ffParsePropFileValues(path, 2, (FFpropquery[]) {
        {"SERVICE=", &result->service},
        {"TYPE=", &result->type},
    }))
        return "Failed to parse /run/systemd/sessions/$XDG_SESSION_ID";

    return NULL;
}
