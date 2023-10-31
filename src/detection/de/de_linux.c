#include "de.h"

#include "common/parsing.h"
#include "common/properties.h"
#include "common/processing.h"
#include "detection/displayserver/displayserver.h"

static void getKDE(FFstrbuf* result, FFDEOptions* options)
{
    ffParsePropFileValues("/usr/share/xsessions/plasmax11.desktop", 1, (FFpropquery[]) {
        {"X-KDE-PluginInfo-Version =", result}
    });
    if(result->length == 0)
        ffParsePropFileData("xsessions/plasma.desktop", "X-KDE-PluginInfo-Version =", result);
    if(result->length == 0)
        ffParsePropFileData("xsessions/plasma5.desktop", "X-KDE-PluginInfo-Version =", result);
    if(result->length == 0)
    {
        ffParsePropFileValues("/usr/share/wayland-sessions/plasma.desktop", 1, (FFpropquery[]) {
            {"X-KDE-PluginInfo-Version =", result}
        });
    }
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

static void getGnome(FFstrbuf* result, FF_MAYBE_UNUSED FFDEOptions* options)
{
    ffParsePropFileData("gnome-shell/org.gnome.Extensions", "version :", result);

    if (result->length == 0)
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
    ffParsePropFileData("applications/cinnamon.desktop", "X-GNOME-Bugzilla-Version =", result);
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

static void getXFCE4(FFstrbuf* result, FFDEOptions* options)
{
    ffParsePropFileData("gtk-doc/html/libxfce4ui/index.html", "<div><p class=\"releaseinfo\">Version", result);

    #ifdef __FreeBSD__
    if(result->length == 0)
    {
        FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/usr/local/share/licenses/");
        if (dirp)
        {
            struct dirent* entry;
            while((entry = readdir(dirp)) != NULL)
            {
                if(!ffStrStartsWith(entry->d_name, "xfce-") || !isdigit(entry->d_name[5]))
                    continue;
                ffStrbufAppendS(result, &entry->d_name[5]);
            }
        }
    }
    #endif

    if(result->length == 0 && options->slowVersionDetection)
    {
        //This is somewhat slow
        ffProcessAppendStdOut(result, (char* const[]){
            "xfce4-session",
            "--version",
            NULL
        });

        ffStrbufSubstrBeforeFirstC(result, '(');
        ffStrbufSubstrAfterFirstC(result, ' ');
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

const char* ffDetectDEVersion(const FFstrbuf* deName, FFstrbuf* result, FFDEOptions* options)
{
    if (ffStrbufEqualS(deName, FF_DE_PRETTY_PLASMA))
        getKDE(result, options);
    else if (ffStrbufEqualS(deName, FF_DE_PRETTY_GNOME) || ffStrbufEqualS(deName, FF_DE_PRETTY_GNOME))
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
    else
        return "Unsupported DE";
    return NULL;
}
