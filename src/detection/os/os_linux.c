#include "os.h"
#include "common/properties.h"
#include "common/parsing.h"

#include <string.h>
#include <stdlib.h>

static inline bool allRelevantValuesSet(const FFOSResult* result)
{
    return result->id.length > 0
        && result->name.length > 0
        && result->prettyName.length > 0
    ;
}

static bool parseFile(const char* fileName, FFOSResult* result)
{
    return ffParsePropFileValues(fileName, 13, (FFpropquery[]) {
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

static void detectOS(FFOSResult* os, const FFinstance* instance)
{
    if(instance->config.osFile.length > 0)
    {
        parseFile(instance->config.osFile.chars, os);
        return;
    }

    if(instance->config.escapeBedrock && parseFile(FASTFETCH_TARGET_DIR_ROOT"/bedrock/etc/bedrock-release", os))
    {
        if(os->id.length == 0)
            ffStrbufAppendS(&os->id, "bedrock");

        if(os->name.length == 0)
            ffStrbufAppendS(&os->name, "Bedrock");

        if(os->prettyName.length == 0)
            ffStrbufAppendS(&os->prettyName, "Bedrock Linux");

        return;
    }

    parseFile(FASTFETCH_TARGET_DIR_ROOT"/etc/os-release", os);
    if(allRelevantValuesSet(os))
        return;

    parseFile(FASTFETCH_TARGET_DIR_USR"/lib/os-release", os);
    if(allRelevantValuesSet(os))
        return;

    parseFile(FASTFETCH_TARGET_DIR_ROOT"/etc/lsb-release", os);
}

void ffDetectOSImpl(FFOSResult* os, const FFinstance* instance)
{
    ffStrbufInit(&os->systemName);
    ffStrbufInit(&os->name);
    ffStrbufInit(&os->prettyName);
    ffStrbufInit(&os->id);
    ffStrbufInit(&os->idLike);
    ffStrbufInit(&os->variant);
    ffStrbufInit(&os->variantID);
    ffStrbufInit(&os->version);
    ffStrbufInit(&os->versionID);
    ffStrbufInit(&os->codename);
    ffStrbufInit(&os->buildID);
    ffStrbufInit(&os->architecture);

    ffStrbufSetS(&os->systemName, instance->state.utsname.sysname);
    ffStrbufSetS(&os->architecture, instance->state.utsname.machine);

    detectOS(os, instance);

    if(ffStrbufIgnCaseCompS(&os->id, "ubuntu") == 0)
        getUbuntuFlavour(os);
}
