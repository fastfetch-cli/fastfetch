#include "os.h"
#include "common/settings.h"

// https://en.wikipedia.org/wiki/Android_version_history
static const struct {
    const char* version;
    const char* codename;
} androidCodenames[] = {
    { "17", "Cinnamon Bun" },
    { "16", "Baklava" },
    { "15", "Vanilla Ice Cream" },
    { "14", "Upside Down Cake" },
    { "13", "Tiramisu" },
    { "12", "Snow Cone" },
    { "11", "Red Velvet Cake" },
    { "10", "Quince Tart" },
    { "9", "Pie" },
    { "8.1", "Oreo" },
    { "8.0", "Oreo" },
    { "7.1", "Nougat" },
    { "7.0", "Nougat" },
    { "6.0", "Marshmallow" },
    { "5.1", "Lollipop" },
    { "5.0", "Lollipop" },
    { "4.4", "KitKat" },
    { "4.3", "Jelly Bean" },
    { "4.2", "Jelly Bean" },
    { "4.1", "Jelly Bean" },
    { "4.0", "Ice Cream Sandwich" },
    { "3.2", "Honeycomb" },
    { "3.1", "Honeycomb" },
    { "3.0", "Honeycomb" },
    { "2.3", "Gingerbread" },
    { "2.2", "Froyo" },
    { "2.1", "Eclair" },
    { "2.0", "Eclair" },
    { "1.6", "Donut" },
    { "1.5", "Cupcake" },
};

void ffDetectOSImpl(FFOSResult* os) {
    ffStrbufSetStatic(&os->name, "Android");

    ffStrbufSetStatic(&os->id, "android");

    ffSettingsGetAndroidProperty("ro.build.version.release", &os->version);

    ffStrbufSet(&os->versionID, &os->version);

    ffSettingsGetAndroidProperty("ro.build.version.codename", &os->codename);

    // On release builds, ro.build.version.codename reports "REL" instead of the real codename.
    // In that case, look up the codename from the version table.
    if (ffStrbufEqualS(&os->codename, "REL")) {
        for (size_t i = 0; i < ARRAY_SIZE(androidCodenames); i++) {
            if (ffStrbufEqualS(&os->version, androidCodenames[i].version)) {
                ffStrbufSetStatic(&os->codename, androidCodenames[i].codename);
                break;
            }
        }
    }

    ffSettingsGetAndroidProperty("ro.build.id", &os->buildID);
}
