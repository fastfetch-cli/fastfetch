#include "de.h"

#include "common/dbus.h"
#include "common/io/io.h"
#include "common/library.h"
#include "common/parsing.h"
#include "common/properties.h"
#include "common/processing.h"
#include "util/binary.h"
#include "util/path.h"
#include "detection/displayserver/displayserver.h"

#include <ctype.h>
#ifdef __FreeBSD__
    #include <paths.h>
    #ifndef _PATH_LOCALBASE
        #define _PATH_LOCALBASE "/usr/local"
    #endif
#elif __OpenBSD__
    #define _PATH_LOCALBASE "/usr/local"
#elif __NetBSD__
    #define _PATH_LOCALBASE "/usr/pkg"
#endif

static void getKDE(FFstrbuf* result, FFDEOptions* options)
{
#ifdef _PATH_LOCALBASE
    ffParsePropFile(_PATH_LOCALBASE "/share/wayland-sessions/plasma.desktop", "X-KDE-PluginInfo-Version =", result);
    if(result->length == 0)
        ffParsePropFile(_PATH_LOCALBASE "/share/xsessions/plasmax11.desktop", "X-KDE-PluginInfo-Version =", result);
#else
    ffParsePropFile(FASTFETCH_TARGET_DIR_USR "/share/wayland-sessions/plasma.desktop", "X-KDE-PluginInfo-Version =", result);
    if(result->length == 0)
        ffParsePropFile(FASTFETCH_TARGET_DIR_USR "/share/xsessions/plasmax11.desktop", "X-KDE-PluginInfo-Version =", result);
#endif

    if(result->length == 0)
        ffParsePropFileData("xsessions/plasma.desktop", "X-KDE-PluginInfo-Version =", result);
    if(result->length == 0)
        ffParsePropFileData("xsessions/plasma5.desktop", "X-KDE-PluginInfo-Version =", result);

    if(result->length == 0)
        ffParsePropFileData("wayland-sessions/plasmawayland.desktop", "X-KDE-PluginInfo-Version =", result);
    if(result->length == 0)
        ffParsePropFileData("wayland-sessions/plasmawayland5.desktop", "X-KDE-PluginInfo-Version =", result);

    if(result->length == 0 && options->slowVersionDetection)
    {
        if (ffProcessAppendStdOut(result, (char* const[]){
            "plasmashell",
            "--version",
            NULL
        }) == NULL) // plasmashell 5.27.5
            ffStrbufSubstrAfterLastC(result, ' ');
    }
}

static const char* getGnomeByDbus(FF_MAYBE_UNUSED FFstrbuf* result)
{
#ifdef FF_HAVE_DBUS
    FFDBusData dbus;
    if (ffDBusLoadData(DBUS_BUS_SESSION, &dbus) != NULL)
        return "ffDBusLoadData() failed";

    ffDBusGetPropertyString(&dbus, "org.gnome.Shell", "/org/gnome/Shell", "org.gnome.Shell", "ShellVersion", result);
    return NULL;
#else // FF_HAVE_DBUS
    return "ffDBusLoadData() failed: dbus support not compiled in";
#endif // FF_HAVE_DBUS
}

static void getGnome(FFstrbuf* result, FF_MAYBE_UNUSED FFDEOptions* options)
{
    getGnomeByDbus(result);

    if (result->length == 0 && options->slowVersionDetection)
    {
        if (ffProcessAppendStdOut(result, (char* const[]){
            "gnome-shell",
            "--version",
            NULL
        }) == NULL) // GNOME Shell 44.1
            ffStrbufSubstrAfterLastC(result, ' ');
    }
}

static void getCinnamon(FFstrbuf* result, FF_MAYBE_UNUSED FFDEOptions* options)
{
    ffStrbufSetS(result, getenv("CINNAMON_VERSION"));

    if (result->length == 0)
        ffParsePropFileData("applications/cinnamon.desktop", "X-GNOME-Bugzilla-Version =", result);

    if (result->length == 0 && options->slowVersionDetection)
    {
        if (ffProcessAppendStdOut(result, (char* const[]){
            "cinnamon",
            "--version",
            NULL
        }) == NULL) // Cinnamon 6.2.2
            ffStrbufSubstrAfterLastC(result, ' ');
    }
}

static void getMate(FFstrbuf* result, FFDEOptions* options)
{
    FF_STRBUF_AUTO_DESTROY major = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY minor = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY micro = ffStrbufCreate();

    ffParsePropFileDataValues("mate-about/mate-version.xml", 3, (FFpropquery[]) {
        {"<platform>", &major},
        {"<minor>", &minor},
        {"<micro>", &micro}
    });

    ffParseSemver(result, &major, &minor, &micro);

    if(result->length == 0 && options->slowVersionDetection)
    {
        ffProcessAppendStdOut(result, (char* const[]){
            "mate-session",
            "--version",
            NULL
        });

        ffStrbufSubstrAfterFirstC(result, ' ');
        ffStrbufTrim(result, ' ');
    }
}

static const char* getXfce4ByLib(FFstrbuf* result)
{
#ifndef FF_DISABLE_DLOPEN
    const char* xfce_version_string(void); // from `xfce4/libxfce4util/xfce-misutils.h
    FF_LIBRARY_LOAD(xfce4util, "dlopen libxfce4util" FF_LIBRARY_EXTENSION "failed", "libxfce4util" FF_LIBRARY_EXTENSION, 7);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(xfce4util, xfce_version_string);
    ffStrbufSetS(result, ffxfce_version_string());
    return NULL;
#else
    FF_UNUSED(result);
    return "dlopen is disabled";
#endif
}

static void getXFCE4(FFstrbuf* result, FFDEOptions* options)
{
    getXfce4ByLib(result);

    if(result->length == 0 && options->slowVersionDetection)
    {
        //This is somewhat slow
        ffProcessAppendStdOut(result, (char* const[]){
            "xfce4-session",
            "--version",
            NULL
        });

        ffStrbufSubstrBeforeFirstC(result, ')');
        ffStrbufSubstrAfterLastC(result, ' ');
        ffStrbufTrim(result, ' ');
    }
}

static void getLXQt(FFstrbuf* result, FFDEOptions* options)
{
    ffParsePropFileData("gconfig/lxqt.pc", "Version:", result);

    if(result->length == 0)
        ffParsePropFileData("cmake/lxqt/lxqt-config.cmake", "set ( LXQT_VERSION", result);
    if(result->length == 0)
        ffParsePropFileData("cmake/lxqt/lxqt-config-version.cmake", "set ( PACKAGE_VERSION", result);

    if(result->length == 0 && options->slowVersionDetection)
    {
        //This is really, really, really slow. Thank you, LXQt developers
        ffProcessAppendStdOut(result, (char* const[]){
            "lxqt-session",
            "-v",
            NULL
        });

        result->length = 0; //don't set '\0' byte
        ffParsePropLines(result->chars , "liblxqt", result);
    }
}

static void getBudgie(FFstrbuf* result, FF_MAYBE_UNUSED FFDEOptions* options)
{
    ffParsePropFileData("budgie/budgie-version.xml", "<str>", result);
}

static void getUnity(FFstrbuf* result, FF_MAYBE_UNUSED FFDEOptions* options)
{
    if (ffParsePropFile("/usr/bin/unity", "parser = OptionParser(version= \"%prog ", result))
        ffStrbufSubstrBeforeFirstC(result, '"');
}

static bool extractTdeVersion(const char* line, uint32_t len, void *userdata)
{
    int count = 0;
    sscanf(line, "R%*d.%*d.%*d%n", &count);
    if (count == 0) return true;

    ffStrbufSetNS((FFstrbuf*) userdata, len, line);
    return false;
}

static const char* getTrinity(FFstrbuf* result, FFDEOptions* options)
{
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
    const char* error = ffFindExecutableInPath("tde-config", &path);
    if (error) return "Failed to find tde-config path";

    ffStrbufSubstrBeforeLastC(&path, '/');
    ffStrbufAppendS(&path, "/../lib/libtdecore.so");

    if (ffBinaryExtractStrings(path.chars, extractTdeVersion, result, strlen("R0.0.0")) == NULL)
        return NULL;

    if (options->slowVersionDetection)
    {
        ffStrbufClear(&path);
        ffProcessAppendStdOut(&path, (char* const[]){
            "tde-config",
            "--version",
            NULL
        });

        ffParsePropLines(path.chars , "TDE: ", result);
        return NULL;
    }

    return "All methods failed";
}


const char* ffDetectDEVersion(const FFstrbuf* deName, FFstrbuf* result, FFDEOptions* options)
{
    if (!instance.config.general.detectVersion) return "Disabled by config";

    if (ffStrbufEqualS(deName, FF_DE_PRETTY_PLASMA))
        getKDE(result, options);
    else if (ffStrbufEqualS(deName, FF_DE_PRETTY_GNOME))
        getGnome(result, options);
    else if (ffStrbufEqualS(deName, FF_DE_PRETTY_CINNAMON))
        getCinnamon(result, options);
    else if (ffStrbufEqualS(deName, FF_DE_PRETTY_XFCE4))
        getXFCE4(result, options);
    else if (ffStrbufEqualS(deName, FF_DE_PRETTY_MATE))
        getMate(result, options);
    else if (ffStrbufEqualS(deName, FF_DE_PRETTY_LXQT))
        getLXQt(result, options);
    else if (ffStrbufEqualS(deName, FF_DE_PRETTY_BUDGIE))
        getBudgie(result, options);
    else if (ffStrbufEqualS(deName, FF_DE_PRETTY_UNITY))
        getUnity(result, options);
    else if (ffStrbufEqualS(deName, "trinity"))
        getTrinity(result, options);
    else
        return "Unsupported DE";
    return NULL;
}
