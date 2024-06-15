#include "os.h"
#include "common/io/io.h"

void ffDetectOSImpl(FFOSResult* os)
{
    if (!ffReadFileBuffer("/etc/release", &os->prettyName))
        return;

    ffStrbufSubstrBeforeFirstC(&os->prettyName, '\n');
    ffStrbufSubstrBeforeLastC(&os->prettyName, '(');
    ffStrbufTrim(&os->prettyName, ' ');

    // OpenIndiana Hipster 2024.04
    uint32_t idx = ffStrbufFirstIndexC(&os->prettyName, ' ');
    ffStrbufSetNS(&os->name, idx, os->prettyName.chars);
    uint32_t idx2 = ffStrbufNextIndexC(&os->prettyName, idx + 1, ' ');
    ffStrbufSetNS(&os->codename, idx2 - idx - 1, os->prettyName.chars + idx + 1);
    ffStrbufSetNS(&os->version, idx2 - idx - 1, os->prettyName.chars + idx2 + 1);
    ffStrbufSetStatic(&os->idLike, "sunos");
}
