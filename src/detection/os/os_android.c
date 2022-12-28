#include "os.h"
#include "common/settings.h"

void ffDetectOSImpl(FFOSResult* os, const FFinstance* instance)
{
    FF_UNUSED(instance);

    ffStrbufInitS(&os->name, "Android");

    ffStrbufInitS(&os->prettyName, "Android");

    ffStrbufInitS(&os->id, "android");

    ffStrbufInit(&os->version);
    ffSettingsGetAndroidProperty("ro.build.version.release", &os->version);

    ffStrbufInit(&os->versionID);
    ffSettingsGetAndroidProperty("ro.build.version.release", &os->versionID);

    ffStrbufInit(&os->codename);
    ffSettingsGetAndroidProperty("ro.build.version.codename", &os->codename);

    ffStrbufInit(&os->buildID);
    ffSettingsGetAndroidProperty("ro.build.id", &os->buildID);

    ffStrbufInitS(&os->systemName, instance->state.utsname.sysname);

    ffStrbufInitS(&os->architecture, instance->state.utsname.machine);

    ffStrbufInit(&os->idLike);
    ffStrbufInit(&os->variant);
    ffStrbufInit(&os->variantID);
}
