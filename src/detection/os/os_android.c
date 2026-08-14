#include "os.h"
#include "common/settings.h"

void ffDetectOSImpl(FFOSResult* os) {
    ffStrbufSetStatic(&os->name, "Android");

    ffStrbufSetStatic(&os->id, "android");

    ffSettingsGetAndroidProperty("ro.build.version.release", &os->version);

    ffStrbufSet(&os->versionID, &os->version);

    unsigned major = 0;
    for (const char* p = os->version.chars; *p >= '0' && *p <= '9'; ++p) {
        major = major * 10 + (unsigned) (*p - '0');
    }

    const char* codename = NULL;
    // https://en.wikipedia.org/wiki/Android_version_history
    // Android 5 is the oldest version supported by Termux
    // clang-format off
    switch (major) {
        case 17: codename = "Cinnamon Bun"; break;
        case 16: codename = "Baklava"; break;
        case 15: codename = "Vanilla Ice Cream"; break;
        case 14: codename = "Upside Down Cake"; break;
        case 13: codename = "Tiramisu"; break;
        case 12: codename = "Snow Cone"; break;
        case 11: codename = "Red Velvet Cake"; break;
        case 10: codename = "Quince Tart"; break;
        case 9: codename = "Pie"; break;
        case 8: codename = "Oreo"; break;
        case 7: codename = "Nougat"; break;
        case 6: codename = "Marshmallow"; break;
        case 5: codename = "Lollipop"; break;
    }
    // clang-format on

    if (codename) {
        ffStrbufSetStatic(&os->codename, codename);
    } else {
        ffSettingsGetAndroidProperty("ro.build.version.codename", &os->codename);
    }

    ffSettingsGetAndroidProperty("ro.build.id", &os->buildID);
}
