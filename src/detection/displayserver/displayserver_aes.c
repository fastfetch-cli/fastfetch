#include "displayserver.h"

#include <mint/cookie.h>

#define COOKIE_nAES /*(const char *)*/0x6e414553L

typedef struct {
    unsigned int    version;
    unsigned int    date;
    unsigned int    time;
    unsigned int    flags;
    struct CNF_VAR  **config;
    unsigned long   unused_2;
} N_AESINFO;

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds)
{
    // cf.
    // https://www.exxosforum.co.uk/atari/mirror/toshyp/00c002.html#N_AESINFO
    // TODO?
    // https://www.exxosforum.co.uk/atari/mirror/toshyp/Application.html#appl_getinfo_str
    long value = -1;
    N_AESINFO *aes;
    if (!Getcookie(COOKIE_nAES, &value)) {
        aes = (N_AESINFO *)value;
        // TODO
        ffStrbufSetStatic(&ds->wmProcessName, "nAES");
        ffStrbufSetStatic(&ds->wmPrettyName, "nAES");
    } else {
        ffStrbufSetStatic(&ds->wmProcessName, "AES");
        ffStrbufSetStatic(&ds->wmPrettyName, "AES");
    }
    ffStrbufSetStatic(&ds->dePrettyName, "GEM");

    //detectDisplays(ds);
}
