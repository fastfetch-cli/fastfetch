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

// Common logic for detecting Armbian image version
FF_MAYBE_UNUSED static bool detectArmbianVersion(FFOSResult* result)
{
    // Possible values `PRETTY_NAME` starts with on Armbian:
    // - `Armbian` for official releases
    // - `Armbian_community` for community releases
    // - `Armbian_Security` for images with kali repo added
    // - `Armbian-unofficial` for an unofficial image built from source, e.g. during development and testing
    if (ffStrbufStartsWithS(&result->prettyName, "Armbian"))
        ffStrbufSetStatic(&result->name, "Armbian");
    else
        return false;
    ffStrbufSet(&result->idLike, &result->id);
    ffStrbufSetS(&result->id, "armbian");
    ffStrbufClear(&result->versionID);
    uint32_t versionStart = ffStrbufFirstIndexC(&result->prettyName, ' ') + 1;
    uint32_t versionEnd = ffStrbufNextIndexC(&result->prettyName, versionStart, ' ');
    ffStrbufSetNS(&result->versionID, versionEnd - versionStart, result->prettyName.chars + versionStart);
    return true;
}

// Returns false if PrettyName should be updated by caller
FF_MAYBE_UNUSED static bool getUbuntuFlavour(FFOSResult* result)
{
    if (detectArmbianVersion(result))
        return true;
    else if(ffStrbufStartsWithS(&result->prettyName, "Linux Lite "))
    {
        ffStrbufSetStatic(&result->name, "Linux Lite");
        ffStrbufSetStatic(&result->id, "linuxlite");
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        ffStrbufSetS(&result->versionID, result->prettyName.chars + strlen("Linux Lite "));
        return true;
    }
    else if(ffStrbufStartsWithS(&result->prettyName, "Rhino Linux "))
    {
        ffStrbufSetStatic(&result->name, "Rhino Linux");
        ffStrbufSetStatic(&result->id, "rhinolinux");
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        ffStrbufSetS(&result->versionID, result->prettyName.chars + strlen("Rhino Linux "));
        return true;
    }
    else if(ffStrbufStartsWithS(&result->prettyName, "VanillaOS "))
    {
        ffStrbufSetStatic(&result->id, "vanilla");
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        return true;
    }

    if (ffPathExists("/usr/bin/lliurex-version", FF_PATHTYPE_FILE))
    {
        ffStrbufSetStatic(&result->name, "LliureX");
        ffStrbufSetStatic(&result->id, "lliurex");
        ffStrbufClear(&result->version);
        if (ffProcessAppendStdOut(&result->version, (char* const[]) {
            "/usr/bin/lliurex-version",
            NULL,
        }) == NULL) // 8.2.2
            ffStrbufTrimRightSpace(&result->version);
        ffStrbufSetF(&result->prettyName, "LliureX %s", result->version.chars);
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        return true;
    }

    const char* xdgConfigDirs = getenv("XDG_CONFIG_DIRS");
    if(!ffStrSet(xdgConfigDirs))
        return false;

    if(ffStrContains(xdgConfigDirs, "kde") || ffStrContains(xdgConfigDirs, "plasma") || ffStrContains(xdgConfigDirs, "kubuntu"))
    {
        ffStrbufSetStatic(&result->name, "Kubuntu");
        ffStrbufSetStatic(&result->id, "kubuntu");
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        return false;
    }

    if(ffStrContains(xdgConfigDirs, "xfce") || ffStrContains(xdgConfigDirs, "xubuntu"))
    {
        ffStrbufSetStatic(&result->name, "Xubuntu");
        ffStrbufSetStatic(&result->id, "xubuntu");
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        return false;
    }

    if(ffStrContains(xdgConfigDirs, "lxqt") || ffStrContains(xdgConfigDirs, "lubuntu"))
    {
        ffStrbufSetStatic(&result->name, "Lubuntu");
        ffStrbufSetStatic(&result->id, "lubuntu");
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        return false;
    }

    if(ffStrContains(xdgConfigDirs, "budgie"))
    {
        ffStrbufSetStatic(&result->name, "Ubuntu Budgie");
        ffStrbufSetStatic(&result->id, "ubuntu-budgie");
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        return false;
    }

    if(ffStrContains(xdgConfigDirs, "cinnamon"))
    {
        ffStrbufSetStatic(&result->name, "Ubuntu Cinnamon");
        ffStrbufSetStatic(&result->id, "ubuntu-cinnamon");
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        return false;
    }

    if(ffStrContains(xdgConfigDirs, "mate"))
    {
        ffStrbufSetStatic(&result->name, "Ubuntu MATE");
        ffStrbufSetStatic(&result->id, "ubuntu-mate");
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        return false;
    }

    if(ffStrContains(xdgConfigDirs, "studio"))
    {
        ffStrbufSetStatic(&result->name, "Ubuntu Studio");
        ffStrbufSetStatic(&result->id, "ubuntu-studio");
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        return false;
    }

    if(ffStrContains(xdgConfigDirs, "sway"))
    {
        ffStrbufSetStatic(&result->name, "Ubuntu Sway");
        ffStrbufSetStatic(&result->id, "ubuntu-sway");
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        return false;
    }

    if(ffStrContains(xdgConfigDirs, "touch"))
    {
        ffStrbufSetStatic(&result->name, "Ubuntu Touch");
        ffStrbufSetStatic(&result->id, "ubuntu-touch");
        ffStrbufSetStatic(&result->idLike, "ubuntu");
        return false;
    }

    return false;
}

FF_MAYBE_UNUSED static void getDebianVersion(FFOSResult* result)
{
    FF_STRBUF_AUTO_DESTROY debianVersion = ffStrbufCreate();
    ffAppendFileBuffer("/etc/debian_version", &debianVersion);
    ffStrbufTrimRightSpace(&debianVersion);
    if (!debianVersion.length) return;
    ffStrbufDestroy(&result->versionID);
    ffStrbufInitMove(&result->versionID, &debianVersion);

    ffStrbufSetF(&result->prettyName, "%s %s (%s)", result->name.chars, result->versionID.chars, result->codename.chars);
}

FF_MAYBE_UNUSED static bool detectDebianDerived(FFOSResult* result)
{
    if (detectArmbianVersion(result))
        return true;
    else if (ffStrbufStartsWithS(&result->name, "Loc-OS"))
    {
        ffStrbufSetStatic(&result->id, "locos");
        ffStrbufSetStatic(&result->idLike, "debian");
        return true;
    }
    else if (ffStrbufEqualS(&result->name, "Parrot Security"))
    {
        // https://github.com/ParrotSec/base-files/blob/c06f6d42ddf8d79564882306576576eddab7d907/etc/os-release
        ffStrbufSetS(&result->id, "parrot");
        ffStrbufSetS(&result->idLike, "debian");
        return true;
    }
    else if (ffStrbufStartsWithS(&result->name, "Lilidog GNU/Linux"))
    {
        // https://github.com/fastfetch-cli/fastfetch/issues/1373
        ffStrbufSetStatic(&result->id, "lilidog");
        ffStrbufSetStatic(&result->idLike, "debian");
        return true;
    }
    else if (access("/usr/bin/pveversion", X_OK) == 0)
    {
        ffStrbufSetStatic(&result->id, "pve");
        ffStrbufSetStatic(&result->idLike, "debian");
        ffStrbufSetStatic(&result->name, "Proxmox VE");
        ffStrbufClear(&result->versionID);
        if (ffProcessAppendStdOut(&result->versionID, (char* const[]) {
            "/usr/bin/dpkg-query",
            "--showformat=${version}",
            "--show",
            "pve-manager",
            NULL,
        }) == NULL) // 8.2.2
            ffStrbufTrimRightSpace(&result->versionID);
        ffStrbufSetF(&result->prettyName, "Proxmox VE %s", result->versionID.chars);
        return true;
    }
    else if (ffPathExists("/etc/rpi-issue", FF_PATHTYPE_FILE))
    {
        // Raspberry Pi OS
        ffStrbufSetStatic(&result->id, "raspbian");
        ffStrbufSetStatic(&result->idLike, "debian");
        ffStrbufSetStatic(&result->name, "Raspberry Pi OS");
        ffStrbufSetStatic(&result->prettyName, "Raspberry Pi OS");
        return true;
    }
    else if (ffPathExists("/boot/dietpi/.version", FF_PATHTYPE_FILE))
    {
        // DietPi
        ffStrbufSetStatic(&result->id, "dietpi");
        ffStrbufSetStatic(&result->name, "DietPi");
        ffStrbufSetStatic(&result->prettyName, "DietPi");
        ffStrbufSetStatic(&result->idLike, "debian");
        FF_STRBUF_AUTO_DESTROY core = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY sub = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY rc = ffStrbufCreate();
        if (ffParsePropFileValues("/boot/dietpi/.version", 3, (FFpropquery[]) {
            {"G_DIETPI_VERSION_CORE=", &core},
            {"G_DIETPI_VERSION_SUB=", &sub},
            {"G_DIETPI_VERSION_RC=", &rc},
        })) ffStrbufAppendF(&result->prettyName, " %s.%s.%s", core.chars, sub.chars, rc.chars);
        return true;
    }
    else if (ffStrbufEndsWithS(&instance.state.platform.sysinfo.release, "+truenas"))
    {
        // TrueNAS Scale
        ffStrbufSetStatic(&result->id, "truenas-scale");
        ffStrbufSetStatic(&result->idLike, "debian");
        ffStrbufSetStatic(&result->name, "TrueNAS Scale");
        ffStrbufSetStatic(&result->prettyName, "TrueNAS Scale");
        return true;
    }
    else
    {
        // Hack for MX Linux. See #847
        FF_STRBUF_AUTO_DESTROY lsbRelease = ffStrbufCreate();
        if (ffAppendFileBuffer("/etc/lsb-release", &lsbRelease) && ffStrbufContainS(&lsbRelease, "DISTRIB_ID=MX"))
        {
            ffStrbufSetStatic(&result->id, "mx");
            ffStrbufSetStatic(&result->idLike, "debian");
            ffStrbufSetStatic(&result->name, "MX");

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

FF_MAYBE_UNUSED static bool detectFedoraVariant(FFOSResult* result)
{
    if (ffStrbufEqualS(&result->variantID, "coreos")
        || ffStrbufEqualS(&result->variantID, "kinoite")
        || ffStrbufEqualS(&result->variantID, "sericea")
        || ffStrbufEqualS(&result->variantID, "silverblue"))
    {
        ffStrbufAppendC(&result->id, '-');
        ffStrbufAppend(&result->id, &result->variantID);
        ffStrbufSetStatic(&result->idLike, "fedora");
        return true;
    }
    return false;
}

static bool detectBedrock(FFOSResult* os)
{
    const char* bedrockRestrict = getenv("BEDROCK_RESTRICT");
    if(bedrockRestrict && bedrockRestrict[0] == '1') return false;
    if(parseOsRelease(FASTFETCH_TARGET_DIR_ROOT "/bedrock" FASTFETCH_TARGET_DIR_ETC "/bedrock-release", os))
    {
        if(os->id.length == 0)
            ffStrbufAppendS(&os->id, "bedrock");

        if(os->name.length == 0)
            ffStrbufAppendS(&os->name, "Bedrock");

        if(os->prettyName.length == 0)
            ffStrbufAppendS(&os->prettyName, "Bedrock Linux");

        parseOsRelease("/bedrock" FASTFETCH_TARGET_DIR_ETC "/os-release", os);
        return true;
    }
    return false;
}

static void detectOS(FFOSResult* os)
{
    #ifdef FF_CUSTOM_OS_RELEASE_PATH
    parseOsRelease(FF_STR(FF_CUSTOM_OS_RELEASE_PATH), os);
        #ifdef FF_CUSTOM_LSB_RELEASE_PATH
        parseLsbRelease(FF_STR(FF_CUSTOM_LSB_RELEASE_PATH), os);
        #endif
    return;
    #endif

    if (detectBedrock(os))
        return;

    // Refer: https://gist.github.com/natefoo/814c5bf936922dad97ff

    parseOsRelease(FASTFETCH_TARGET_DIR_ETC "/os-release", os);
    if (os->id.length == 0 || os->version.length == 0 || os->prettyName.length == 0 || os->codename.length == 0)
        parseLsbRelease(FASTFETCH_TARGET_DIR_ETC "/lsb-release", os);
    if (os->id.length == 0 || os->name.length == 0 || os->prettyName.length == 0)
        parseOsRelease(FASTFETCH_TARGET_DIR_USR "/lib/os-release", os);
    if (os->id.length == 0 && os->name.length == 0 && os->prettyName.length == 0)
    {
        // HarmonyOS has no os-release file
        if (ffStrbufEqualS(&instance.state.platform.sysinfo.name, "HarmonyOS"))
        {
            ffStrbufSetS(&os->id, "harmonyos");
            ffStrbufSetS(&os->idLike, "harmonyos");
            ffStrbufSetS(&os->name, "HarmonyOS");
            ffStrbufSetS(&os->prettyName, "HarmonyOS");
        }
    }
}

void ffDetectOSImpl(FFOSResult* os)
{
    detectOS(os);

    #if __linux__ || __GNU__
    if(ffStrbufEqualS(&os->id, "ubuntu"))
    {
        if (!getUbuntuFlavour(os))
        {
            if (!ffStrbufEndsWithS(&os->prettyName, " (development branch)"))
                ffStrbufSetF(&os->prettyName, "%s %s", os->name.chars, os->version.chars); // os->version contains code name
        }
    }
    else if(ffStrbufEqualS(&os->id, "debian"))
    {
        if (!detectDebianDerived(os))
            getDebianVersion(os);
    }
    else if(ffStrbufEqualS(&os->id, "fedora"))
        detectFedoraVariant(os);
    else if(ffStrbufEqualS(&os->id, "linuxmint"))
    {
        if (ffStrbufEqualS(&os->name, "LMDE"))
        {
            ffStrbufSetS(&os->id, "lmde");
            ffStrbufSetS(&os->idLike, "linuxmint");
        }
    }
    #endif
}
