#include "lm.h"
#include "common/properties.h"
#include "common/dbus.h"
#include "common/processing.h"
#include "detection/displayserver/displayserver.h"

#include <unistd.h>

#define FF_SYSTEMD_SESSIONS_PATH "/var/run/systemd/sessions/"
#define FF_SYSTEMD_USERS_PATH "/run/systemd/users/"

static const char* getGdmVersion(FFstrbuf* version)
{
    const char* error = ffProcessAppendStdOut(version, (char* const[]) {
        "gdm",
        "--version",
        NULL
    });
    if (error)
        return error;

    // GDM 44.1
    ffStrbufSubstrAfterFirstC(version, ' ');
    return NULL;
}

#ifdef FF_HAVE_ZLIB
#include "common/library.h"
#include <stdlib.h>
#include <zlib.h>

static const char* getSddmVersion(FFstrbuf* version)
{
    FF_LIBRARY_LOAD(zlib, &instance.config.libZ, "dlopen libz failed", "libz" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(zlib, gzopen);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(zlib, gzread);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(zlib, gzerror);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(zlib, gztell);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(zlib, gzrewind);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(zlib, gzclose);

    gzFile file = ffgzopen("/usr/share/man/man1/sddm.1.gz", "rb");
    if (file == Z_NULL)
        return "ffgzopen(\"/usr/share/man/man1/sddm.1.gz\", \"rb\") failed";

    ffStrbufEnsureFree(version, 2047);
    memset(version->chars, 0, version->allocated);
    int size = ffgzread(file, version->chars, version->allocated - 1);
    ffgzclose(file);

    if (size <= 0)
        return "ffgzread(file, version->chars, version->length) failed";

    version->length = (uint32_t) size;
    uint32_t index = ffStrbufFirstIndexS(version, ".TH ");
    if (index == version->length)
    {
        ffStrbufClear(version);
        return ".TH is not found";
    }

    ffStrbufSubstrBefore(version, ffStrbufNextIndexC(version, index, '\n'));
    ffStrbufSubstrAfter(version, index + strlen(".TH "));

    // "SDDM" 1 "May 2014" "sddm 0.20.0" "sddm"
    ffStrbufSubstrBeforeLastC(version, ' ');
    ffStrbufTrimRight(version, '"');
    ffStrbufSubstrAfterLastC(version, ' ');

    return NULL;
}
#else
static const char* getSddmVersion(FF_MAYBE_UNUSED FFstrbuf* version)
{
    return "Fastfetch is built without libz support";
}
#endif

static const char* getXfwmVersion(FFstrbuf* version)
{
    const char* error = ffProcessAppendStdOut(version, (char* const[]) {
        "xfwm4",
        "--version",
        NULL
    });
    if (error)
        return error;

    //         This is xfwm4 version 4.18.0 (revision 7e7473c5b) for Xfce 4.18...
    ffStrbufSubstrAfterFirstS(version, "version ");
    ffStrbufSubstrBeforeFirstC(version, ' ');

    return NULL;
}

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

    if (ffStrbufStartsWithS(&result->service, "gdm"))
        getGdmVersion(&result->version);
    else if (ffStrbufStartsWithS(&result->service, "sddm"))
        getSddmVersion(&result->version);
    else if (ffStrbufStartsWithS(&result->service, "xfwm"))
        getXfwmVersion(&result->version);

    // Correct char cases
    if (ffStrbufIgnCaseEqualS(&result->type, FF_WM_PROTOCOL_WAYLAND))
        ffStrbufSetS(&result->type, FF_WM_PROTOCOL_WAYLAND);
    else if (ffStrbufIgnCaseEqualS(&result->type, FF_WM_PROTOCOL_X11))
        ffStrbufSetS(&result->type, FF_WM_PROTOCOL_X11);
    else if (ffStrbufIgnCaseEqualS(&result->type, FF_WM_PROTOCOL_TTY))
        ffStrbufSetS(&result->type, FF_WM_PROTOCOL_TTY);

    return NULL;
}
