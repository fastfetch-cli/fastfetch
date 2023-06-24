#include "os.h"
#include "common/settings.h"

void ffDetectOSImpl(FFOSResult* os)
{
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

    ffStrbufInit(&os->idLike);
    ffStrbufInit(&os->variant);
    ffStrbufInit(&os->variantID);
}
