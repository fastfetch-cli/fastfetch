#include "os.h"
#include "common/properties.h"
#include "common/parsing.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "util/stringUtils.h"

#include <string.h>
#include <stdlib.h>

#define FF_STR_INDIR(x) #x
#define FF_STR(x) FF_STR_INDIR(x)

static inline bool allRelevantValuesSet(const FFOSResult* result)
{
    return result->id.length > 0
        && result->name.length > 0
        && result->prettyName.length > 0
    ;
}

static bool parseLsbRelease(const char* fileName, FFOSResult* result)
{
    return ffParsePropFileValues(fileName, 4, (FFpropquery[]) {
        {"DISTRIB_ID =", &result->id},
        {"DISTRIB_DESCRIPTION =", &result->prettyName},
        {"DISTRIB_RELEASE =", &result->version},
        {"DISTRIB_CODENAME =", &result->codename},
    });
}

static bool parseOsRelease(const char* fileName, FFOSResult* result)
{
    return ffParsePropFileValues(fileName, 11, (FFpropquery[]) {
        {"PRETTY_NAME =", &result->prettyName},
        {"NAME =", &result->name},
        {"ID =", &result->id},
        {"ID_LIKE =", &result->idLike},
        {"VARIANT =", &result->variant},
        {"VARIANT_ID =", &result->variantID},
        {"VERSION =", &result->version},
        {"VERSION_ID =", &result->versionID},
        {"VERSION_CODENAME =", &result->codename},
        {"CODENAME =", &result->codename},
        {"BUILD_ID =", &result->buildID},
    });
}

static void getUbuntuFlavour(FFOSResult* result)
{
    const char* xdgConfigDirs = getenv("XDG_CONFIG_DIRS");
    if(!ffStrSet(xdgConfigDirs))
        return;

    if(ffStrContains(xdgConfigDirs, "kde") || ffStrContains(xdgConfigDirs, "plasma") || ffStrContains(xdgConfigDirs, "kubuntu"))
    {
        ffStrbufSetS(&result->name, "Kubuntu");
        ffStrbufSetS(&result->prettyName, "Kubuntu");
        ffStrbufSetS(&result->id, "kubuntu");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(ffStrContains(xdgConfigDirs, "xfce") || ffStrContains(xdgConfigDirs, "xubuntu"))
    {
        ffStrbufSetS(&result->name, "Xubuntu");
        ffStrbufSetS(&result->prettyName, "Xubuntu");
        ffStrbufSetS(&result->id, "xubuntu");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(ffStrContains(xdgConfigDirs, "lxde") || ffStrContains(xdgConfigDirs, "lubuntu"))
    {
        ffStrbufSetS(&result->name, "Lubuntu");
        ffStrbufSetS(&result->prettyName, "Lubuntu");
        ffStrbufSetS(&result->id, "lubuntu");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(ffStrContains(xdgConfigDirs, "budgie"))
    {
        ffStrbufSetS(&result->name, "Ubuntu Budgie");
        ffStrbufSetS(&result->prettyName, "Ubuntu Budgie");
        ffStrbufSetS(&result->id, "ubuntu-budgie");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(ffStrContains(xdgConfigDirs, "cinnamon"))
    {
        ffStrbufSetS(&result->name, "Ubuntu Cinnamon");
        ffStrbufSetS(&result->prettyName, "Ubuntu Cinnamon");
        ffStrbufSetS(&result->id, "ubuntu-cinnamon");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(ffStrContains(xdgConfigDirs, "mate"))
    {
        ffStrbufSetS(&result->name, "Ubuntu MATE");
        ffStrbufSetS(&result->prettyName, "Ubuntu MATE");
        ffStrbufSetS(&result->id, "ubuntu-mate");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(ffStrContains(xdgConfigDirs, "studio"))
    {
        ffStrbufSetS(&result->name, "Ubuntu Studio");
        ffStrbufSetS(&result->prettyName, "Ubuntu Studio");
        ffStrbufSetS(&result->id, "ubuntu-studio");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(ffStrContains(xdgConfigDirs, "sway"))
    {
        ffStrbufSetS(&result->name, "Ubuntu Sway");
        ffStrbufSetS(&result->prettyName, "Ubuntu Sway");
        ffStrbufSetS(&result->id, "ubuntu-sway");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(ffStrContains(xdgConfigDirs, "touch"))
    {
        ffStrbufSetS(&result->name, "Ubuntu Touch");
        ffStrbufSetS(&result->prettyName, "Ubuntu Touch");
        ffStrbufSetS(&result->id, "ubuntu-touch");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }

    if(ffStrContains(xdgConfigDirs, "lliurex"))
    {
        ffStrbufSetS(&result->name, "LliureX");
        ffStrbufSetS(&result->prettyName, "LliureX");
        ffStrbufSetS(&result->id, "lliurex");
        ffStrbufSetS(&result->idLike, "ubuntu");
        return;
    }
}

static void getDebianVersion(FFOSResult* result)
{
    FF_STRBUF_AUTO_DESTROY debianVersion = ffStrbufCreate();
    ffAppendFileBuffer("/etc/debian_version", &debianVersion);
    ffStrbufTrimRightSpace(&debianVersion);
    if (!debianVersion.length) return;
    ffStrbufSet(&result->version, &debianVersion);
    ffStrbufSet(&result->versionID, &debianVersion);
}

static bool detectDebianDerived(FFOSResult* result)
{
    if (ffStrbufStartsWithS(&result->prettyName, "Armbian ")) // Armbian 24.2.1 bookworm
    {
        ffStrbufSetS(&result->name, "Armbian");
        ffStrbufSetS(&result->id, "armbian");
        ffStrbufSetS(&result->idLike, "debian");
        ffStrbufClear(&result->versionID);
        uint32_t versionStart = ffStrbufFirstIndexC(&result->prettyName, ' ') + 1;
        uint32_t versionEnd = ffStrbufNextIndexC(&result->prettyName, versionStart, ' ');
        ffStrbufSetNS(&result->versionID, versionEnd - versionStart, result->prettyName.chars + versionStart);
        return true;
    }
    else if (ffStrbufStartsWithS(&result->name, "Loc-OS"))
    {
        ffStrbufSetS(&result->id, "locos");
        ffStrbufSetS(&result->idLike, "debian");
        return true;
    }
    else if (ffStrbufEqualS(&result->name, "Parrot Security"))
    {
        // https://github.com/ParrotSec/base-files/blob/c06f6d42ddf8d79564882306576576eddab7d907/etc/os-release
        ffStrbufSetS(&result->id, "parrot");
        ffStrbufSetS(&result->idLike, "debian");
        return true;
    }
    else if (access("/usr/bin/pveversion", X_OK) == 0)
    {
        ffStrbufSetS(&result->id, "pve");
        ffStrbufSetS(&result->idLike, "debian");
        ffStrbufSetS(&result->name, "Proxmox VE");
        ffStrbufClear(&result->versionID);
        if (ffProcessAppendStdOut(&result->versionID, (char* const[]) {
            "/usr/bin/pveversion",
            NULL,
        }) == NULL) // pve-manager/8.2.2/9355359cd7afbae4 (running kernel: 6.8.4-2-pve)
        {
            ffStrbufSubstrBeforeLastC(&result->versionID, '/');
            ffStrbufSubstrAfterFirstC(&result->versionID, '/');
        }
        ffStrbufSetF(&result->prettyName, "Proxmox VE %s", result->versionID.chars);
        return true;
    }
    else
    {
        // Hack for MX Linux. See #847
        FF_STRBUF_AUTO_DESTROY lsbRelease = ffStrbufCreate();
        if (ffAppendFileBuffer("/etc/lsb-release", &lsbRelease) && ffStrbufContainS(&lsbRelease, "DISTRIB_ID=MX"))
        {
            ffStrbufSetS(&result->id, "mx");
            ffStrbufSetS(&result->idLike, "debian");
            ffStrbufSetS(&result->name, "MX");

            ffStrbufClear(&result->version);
            ffParsePropLines(lsbRelease.chars, "DISTRIB_RELEASE=", &result->version);
            ffStrbufSet(&result->versionID, &result->version);

            ffStrbufClear(&result->codename);
            ffParsePropLines(lsbRelease.chars, "DISTRIB_CODENAME=", &result->codename);

            ffStrbufClear(&result->prettyName);
            ffParsePropLines(lsbRelease.chars, "DISTRIB_DESCRIPTION=", &result->prettyName);
            return true;
        }
    }
    return false;
}

static void detectOS(FFOSResult* os)
{
    #ifdef FF_CUSTOM_OS_RELEASE_PATH
    parseOsRelease(FF_STR(FF_CUSTOM_OS_RELEASE_PATH), os);
    parseLsbRelease(FF_STR(FF_CUSTOM_OS_RELEASE_PATH), os);
    return;
    #endif

    if(instance.config.general.escapeBedrock && parseOsRelease(FASTFETCH_TARGET_DIR_ROOT "/bedrock" FASTFETCH_TARGET_DIR_ETC "/bedrock-release", os))
    {
        if(os->id.length == 0)
            ffStrbufAppendS(&os->id, "bedrock");

        if(os->name.length == 0)
            ffStrbufAppendS(&os->name, "Bedrock");

        if(os->prettyName.length == 0)
            ffStrbufAppendS(&os->prettyName, "Bedrock Linux");

        if(parseOsRelease("/bedrock" FASTFETCH_TARGET_DIR_ETC "/os-release", os) && allRelevantValuesSet(os))
            return;
    }

    // Refer: https://gist.github.com/natefoo/814c5bf936922dad97ff

    if((parseOsRelease(FASTFETCH_TARGET_DIR_ETC "/os-release", os) ||
        parseLsbRelease(FASTFETCH_TARGET_DIR_ETC "/lsb-release", os)) &&
        allRelevantValuesSet(os))
        return;

    parseOsRelease(FASTFETCH_TARGET_DIR_USR "/lib/os-release", os);
}

void ffDetectOSImpl(FFOSResult* os)
{
    detectOS(os);

    if(ffStrbufIgnCaseEqualS(&os->id, "ubuntu"))
        getUbuntuFlavour(os);
    else if(ffStrbufIgnCaseEqualS(&os->id, "debian"))
    {
        if (!detectDebianDerived(os))
            getDebianVersion(os);
    }
}
