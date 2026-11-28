#include "common/keenetic/version.h"
#include "common/processing.h"
#include "common/FFstrbuf.h"
#include <string.h>
#include <ctype.h>

static const char* getValueAfter(const char* line, const char* key)
{
    const char* p = strstr(line, key);
    if (!p) return NULL;

    p += strlen(key);
    while (*p == ' ' || *p == '\t') ++p;
    return *p ? p : NULL;
}

const FFKeeneticInfo* ffDetectKeenetic(void)
{
    static FFKeeneticInfo info;
    static bool initialized = false;

    if (initialized)
        return &info;

    initialized = true;
    memset(&info, 0, sizeof(info));
    ffStrbufInit(&info.vendor);
    ffStrbufInit(&info.title);
    ffStrbufInit(&info.model);

    if (!ffPathExists("/bin/ndmc", FF_PATHTYPE_FILE))
        return &info;

    FF_STRBUF_AUTO_DESTROY output = ffStrbufCreate();
    if (ffProcessAppendStdOut(&output, (char* const[]){
            "/bin/ndmc", "-c", "show version", NULL
        }) != NULL || output.length == 0)
    {
        return &info;
    }

    char* line = output.chars;
    while (line && *line)
    {
        char* next = strchr(line, '\n');
        if (next) *next = '\0';

        const char* v = getValueAfter(line, "vendor:");
        if (v) ffStrbufSetS(&info.vendor, v);

        const char* t = getValueAfter(line, "title:");
        if (t) ffStrbufSetS(&info.title, t);

        const char* m = getValueAfter(line, "model:");
        if (m) ffStrbufSetS(&info.model, m);

        const char* f = getValueAfter(line, "manufacturer:");
        if (f) ffStrbufSetS(&info.vendor_full, f);

        const char* r = getValueAfter(line, "release:");
        if (m) ffStrbufSetS(&info.version, r);

        line = next ? next + 1 : NULL;
    }

    info.available = (info.vendor.length > 0);
    info.id = "keenetic";
    info.idLike = "keeneticos";
    info.family = "linux";

    return &info;
}

void ffKeeneticFormatOS(FFstrbuf* result)
{
    const FFKeeneticInfo* k = ffDetectKeenetic();
    if (!k->available) return;

    if (k->title.length > 0)
        ffStrbufSetF(result, "%s OS", k->vendor.chars);
    else
        ffStrbufSetS(result, k->vendor.chars);
}

void ffKeeneticFormatOSFull(FFstrbuf* result)
{
    const FFKeeneticInfo* k = ffDetectKeenetic();
    if (!k->available) return;

    if (k->title.length > 0)
        ffStrbufSetF(result, "%s OS %s", k->vendor.chars, k->title.chars);
    else
        ffStrbufSetS(result, k->vendor.chars);
}

void ffKeeneticFormatHost(FFstrbuf* result)
{
    const FFKeeneticInfo* k = ffDetectKeenetic();
    if (!k->available) return;

    if (k->model.length > 0)
        ffStrbufSetF(result, "%s %s", k->vendor.chars, k->model.chars);
    else
        ffStrbufSetS(result, k->vendor.chars);
}
