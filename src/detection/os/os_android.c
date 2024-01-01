#include "os.h"
#include "common/settings.h"

void ffDetectOSImpl(FFOSResult* os)
{
    ffStrbufSetStatic(&os->name, "Android");

    ffStrbufSetStatic(&os->prettyName, "Android");

    ffStrbufSetStatic(&os->id, "android");

    ffSettingsGetAndroidProperty("ro.build.version.release", &os->version);

    ffSettingsGetAndroidProperty("ro.build.version.release", &os->versionID);

    ffSettingsGetAndroidProperty("ro.build.version.codename", &os->codename);

    ffSettingsGetAndroidProperty("ro.build.id", &os->buildID);
}
