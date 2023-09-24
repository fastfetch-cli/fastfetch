#include "os.h"
#include "common/properties.h"
#include "common/parsing.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

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

    if(strstr(xdgConfigDirs, "cinnamon") != NULL)
    {
        ffStrbufSetS(&result->name, "Ubuntu Cinnamon");
        ffStrbufSetS(&result->prettyName, "Ubuntu Cinnamon");
        ffStrbufSetS(&result->id, "ubuntu-cinnamon");
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

    if(strstr(xdgConfigDirs, "sway") != NULL)
    {
        ffStrbufSetS(&result->name, "Ubuntu Sway");
        ffStrbufSetS(&result->prettyName, "Ubuntu Sway");
        ffStrbufSetS(&result->id, "ubuntu-sway");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(strstr(xdgConfigDirs, "touch") != NULL)
    {
        ffStrbufSetS(&result->name, "Ubuntu Touch");
        ffStrbufSetS(&result->prettyName, "Ubuntu Touch");
        ffStrbufSetS(&result->id, "ubuntu-touch");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }
}

static void getDebianVersion(FFOSResult* result)
{
    FF_STRBUF_AUTO_DESTROY debianVersion = ffStrbufCreate();
    ffAppendFileBuffer("/etc/debian_version", &debianVersion);
    if (debianVersion.length)
        ffStrbufSet(&result->version, &debianVersion);
}

static void detectOS(FFOSResult* os)
{
    if(instance.config.osFile.length > 0)
    {
        parseFile(instance.config.osFile.chars, os);
        return;
    }

    if(instance.config.escapeBedrock && parseFile(FASTFETCH_TARGET_DIR_ROOT"/bedrock"FASTFETCH_TARGET_DIR_ETC"/bedrock-release", os))
    {
        if(os->id.length == 0)
            ffStrbufAppendS(&os->id, "bedrock");

        if(os->name.length == 0)
            ffStrbufAppendS(&os->name, "Bedrock");

        if(os->prettyName.length == 0)
            ffStrbufAppendS(&os->prettyName, "Bedrock Linux");

        return;
    }

    parseFile(FASTFETCH_TARGET_DIR_ETC"/os-release", os);
    if(allRelevantValuesSet(os))
        return;

    parseFile(FASTFETCH_TARGET_DIR_USR"/lib/os-release", os);
    if(allRelevantValuesSet(os))
        return;

    parseFile(FASTFETCH_TARGET_DIR_ETC"/lsb-release", os);
}

void ffDetectOSImpl(FFOSResult* os)
{
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

    detectOS(os);

    if(ffStrbufIgnCaseEqualS(&os->id, "ubuntu"))
        getUbuntuFlavour(os);
    else if(ffStrbufIgnCaseEqualS(&os->id, "debian"))
        getDebianVersion(os);
}
