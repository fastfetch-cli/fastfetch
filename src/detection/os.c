#include "fastfetch.h"

#include <pthread.h>

const FFOSResult* ffDetectOS(FFinstance* instance)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static FFOSResult result;
    static bool init = false;
    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    ffStrbufInit(&result.systemName);
    ffStrbufInit(&result.name);
    ffStrbufInit(&result.prettyName);
    ffStrbufInit(&result.id);
    ffStrbufInit(&result.idLike);
    ffStrbufInit(&result.variant);
    ffStrbufInit(&result.variantID);
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.versionID);
    ffStrbufInit(&result.codename);
    ffStrbufInit(&result.buildID);
    ffStrbufInit(&result.architecture);
    ffStrbufInit(&result.error);

    ffStrbufSetS(&result.systemName, instance->state.utsname.sysname);
    ffStrbufSetS(&result.architecture, instance->state.utsname.machine);

#if !__ANDROID__
    bool isOsRelease = true;
    FILE* file = fopen("/etc/os-release", "r");

    if(file == NULL)
        file = fopen("/usr/lib/os-release", "r");

    if(file == NULL)
    {
        file = fopen("/etc/lsb-release", "r");
        isOsRelease = false;
    }

    ffStrbufInitA(&result.error, 64);
    if(file == NULL)
    {
        ffStrbufAppendS(&result.error, "couldn't read /etc/os-release nor /usr/lib/os-release nor /etc/lsb-release");
        return &result;
    }

    char* line = NULL;
    size_t len = 0;

    // Documentation of the fields:
    // https://www.freedesktop.org/software/systemd/man/os-release.html
    while (getline(&line, &len, file) != -1)
    {
        if(isOsRelease)
        {
            ffGetPropValue(line, "NAME =", &result.name);
            ffGetPropValue(line, "PRETTY_NAME =", &result.prettyName);
            ffGetPropValue(line, "ID =", &result.id);
            ffGetPropValue(line, "ID_LIKE =", &result.idLike);
            ffGetPropValue(line, "VARIANT =", &result.variant);
            ffGetPropValue(line, "VARIANT_ID =", &result.variantID);
            ffGetPropValue(line, "VERSION =", &result.version);
            ffGetPropValue(line, "VERSION_ID =", &result.versionID);
            ffGetPropValue(line, "VERSION_CODENAME =", &result.codename);
            ffGetPropValue(line, "BUILD_ID =", &result.buildID);
        }
        else
        {
            ffGetPropValue(line, "DISTRIB_ID =", &result.id);
            ffGetPropValue(line, "DISTRIB_RELEASE =", &result.version);
            ffGetPropValue(line, "DISTRIB_DESCRIPTION =", &result.prettyName);
        }
    }

    if(line != NULL)
        free(line);

    fclose(file);
#else
    ffStrbufSetS(&result.name, "Android");
    ffStrbufSetS(&result.id, "android");
    ffSettingsGetAndroidProperty("ro.build.version.release", &result.versionID);
    ffSettingsGetAndroidProperty("ro.build.version.release", &result.version);
    ffSettingsGetAndroidProperty("ro.build.version.codename", &result.codename);
    ffSettingsGetAndroidProperty("ro.build.id", &result.buildID);
#endif

    pthread_mutex_unlock(&mutex);

    return &result;
}
