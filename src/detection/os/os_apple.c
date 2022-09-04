#include "os.h"
#include "common/settings.h"

void ffDetectOSImpl(FFOSResult* os, const FFinstance* instance)
{
    FF_UNUSED(instance);
    ffStrbufInitA(&os->name, 0);
    ffStrbufInitA(&os->prettyName, 0);
    ffStrbufInitA(&os->id, 0);
    ffStrbufInitA(&os->version, 0);
    ffStrbufInitA(&os->versionID, 0);
    ffStrbufInitA(&os->codename, 0);
    ffStrbufInitA(&os->buildID, 0);
    ffStrbufInitA(&os->systemName, 0);
    ffStrbufInitA(&os->architecture, 0);
    ffStrbufInitA(&os->idLike, 0);
    ffStrbufInitA(&os->variant, 0);
    ffStrbufInitA(&os->variantID, 0);
}
