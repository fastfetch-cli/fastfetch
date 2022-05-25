#include "fastfetch.h"

#include <string.h>
#include <pthread.h>

#if !defined(__ANDROID__)

static void parseFile(const char* fileName, FFOSResult* result)
{
    if(result->id.length > 0 && result->name.length > 0 && result->prettyName.length > 0)
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

static void getUbuntuFlavour(FFOSResult* result)
{
    const char* xdgConfigDirs = getenv("XDG_CONFIG_DIRS");
    if(!ffStrSet(xdgConfigDirs))
        return;

    if(strstr(xdgConfigDirs, "kde") != NULL || strstr(xdgConfigDirs, "plasma") != NULL)
    {
        ffStrbufSetS(&result->name, "Kubuntu");
        ffStrbufSetS(&result->prettyName, "Kubuntu");
        ffStrbufSetS(&result->id, "kubuntu");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(strstr(xdgConfigDirs, "xfce") != NULL || strstr(xdgConfigDirs, "xubuntu") != NULL)
    {
        ffStrbufSetS(&result->name, "Xubuntu");
        ffStrbufSetS(&result->prettyName, "Xubuntu");
        ffStrbufSetS(&result->id, "xubuntu");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(strstr(xdgConfigDirs, "lxde") != NULL || strstr(xdgConfigDirs, "lubuntu") != NULL)
    {
        ffStrbufSetS(&result->name, "Lubuntu");
        ffStrbufSetS(&result->prettyName, "Lubuntu");
        ffStrbufSetS(&result->id, "lubuntu");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(strstr(xdgConfigDirs, "budgie") != NULL)
    {
        ffStrbufSetS(&result->name, "Ubuntu Budgie");
        ffStrbufSetS(&result->prettyName, "Ubuntu Budgie");
        ffStrbufSetS(&result->id, "ubuntu-budgie");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(strstr(xdgConfigDirs, "mate") != NULL)
    {
        ffStrbufSetS(&result->name, "Ubuntu MATE");
        ffStrbufSetS(&result->prettyName, "Ubuntu MATE");
        ffStrbufSetS(&result->id, "ubuntu-mate");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(strstr(xdgConfigDirs, "studio") != NULL)
    {
        ffStrbufSetS(&result->name, "Ubuntu Studio");
        ffStrbufSetS(&result->prettyName, "Ubuntu Studio");
        ffStrbufSetS(&result->id, "ubuntu-studio");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }
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
    else if(ffFileExists(FASTFETCH_TARGET_DIR_ROOT"/bedrock/etc/bedrock-release", S_IFDIR)) {
        parseFile(FASTFETCH_TARGET_DIR_ROOT"/bedrock/etc/bedrock-release", &result);

        if(result.id.length == 0)
            ffStrbufAppendS(&result.id, "bedrock");

        if(result.name.length == 0)
            ffStrbufAppendS(&result.name, "Bedrock");

        if(result.prettyName.length == 0)
            ffStrbufAppendS(&result.prettyName, "Bedrock Linux");
    }
    else
    {
        parseFile(FASTFETCH_TARGET_DIR_ROOT"/etc/os-release", &result);
        parseFile(FASTFETCH_TARGET_DIR_USR"/lib/os-release", &result);
        parseFile(FASTFETCH_TARGET_DIR_ROOT"/etc/lsb-release", &result);
    }

    if(ffStrbufIgnCaseCompS(&result.id, "ubuntu") == 0)
        getUbuntuFlavour(&result);

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
