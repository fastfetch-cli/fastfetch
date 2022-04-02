#include "fastfetch.h"

#include <pthread.h>

#if !defined(__ANDROID__)

static void parseFile(const char* fileName, FFOSResult* result)
{
    if(result->name.length > 0 && result->prettyName.length > 0)
        return;

    ffParsePropFileValues(fileName, 13, (FFpropquery[]) {
        {"NAME =", &result->name},
        {"DISTRIB_DESCRIPTION =", &result->prettyName},
        {"PRETTY_NAME =", &result->prettyName},
        {"DISTRIB_ID =", &result->id},
        {"ID =", &result->id},
        {"ID_LIKE =", &result->idLike},
        {"VARIANT =", &result->variant},
        {"VARIANT_ID =", &result->variantID},
        {"DISTRIB_RELEASE =", &result->version},
        {"VERSION =", &result->version},
        {"VERSION_ID =", &result->versionID},
        {"VERSION_CODENAME =", &result->codename},
        {"BUILD_ID =", &result->buildID}
    });
}

#endif

const FFOSResult* ffDetectOS(const FFinstance* instance)
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

    ffStrbufSetS(&result.systemName, instance->state.utsname.sysname);
    ffStrbufSetS(&result.architecture, instance->state.utsname.machine);

#if !__ANDROID__
    if(instance->config.osFile.length > 0)
    {
        parseFile(instance->config.osFile.chars, &result);
    }
    else
    {
        parseFile("/etc/os-release", &result);
        parseFile("/usr/lib/os-release", &result);
        parseFile("/etc/lsb-release", &result);
    }
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
