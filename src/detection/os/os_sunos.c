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
    ffStrbufSetNS(&os->id, idx, os->prettyName.chars);
    ffStrbufSetStatic(&os->idLike, "sunos");
}
