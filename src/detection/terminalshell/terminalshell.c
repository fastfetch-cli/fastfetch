#include "fastfetch.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "common/properties.h"
#include "util/stringUtils.h"
#include "util/binary.h"

#include <ctype.h>
#ifdef __FreeBSD__
    #include <paths.h>
#elif __OpenBSD__
    #define _PATH_LOCALBASE "/usr/local"
#endif

#ifdef _WIN32

#include "util/mallocHelper.h"

#include <winver.h>

static bool getFileVersion(const char* exePath, FFstrbuf* version)
{
    DWORD handle;
    DWORD size = GetFileVersionInfoSizeA(exePath, &handle);
    if(size > 0)
    {
        FF_AUTO_FREE void* versionData = malloc(size);
        if(GetFileVersionInfoA(exePath, handle, size, versionData))
        {
            VS_FIXEDFILEINFO* verInfo;
            UINT len;
            if(VerQueryValueW(versionData, L"\\", (void**)&verInfo, &len) && len && verInfo->dwSignature == 0xFEEF04BD)
            {
                ffStrbufAppendF(version, "%u.%u.%u.%u",
                    (unsigned)(( verInfo->dwFileVersionMS >> 16 ) & 0xffff),
                    (unsigned)(( verInfo->dwFileVersionMS >>  0 ) & 0xffff),
                    (unsigned)(( verInfo->dwFileVersionLS >> 16 ) & 0xffff),
                    (unsigned)(( verInfo->dwFileVersionLS >>  0 ) & 0xffff)
                );
                return true;
            }
        }
    }

    return false;
}

#endif

static bool getExeVersionRaw(FFstrbuf* exe, FFstrbuf* version)
{
    return ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    }) == NULL;
}

static bool getExeVersionGeneral(FFstrbuf* exe, FFstrbuf* version)
{
    if(!getExeVersionRaw(exe, version))
        return false;

    ffStrbufSubstrAfterFirstC(version, ' ');
    ffStrbufSubstrBeforeFirstC(version, ' ');
    return true;
}

static bool extractBashVersion(const char* line, FF_MAYBE_UNUSED uint32_t len, void *userdata)
{
    if (!ffStrStartsWith(line, "@(#)Bash version ")) return true;
    const char* start = line + strlen("@(#)Bash version ");
    const char* end = strchr(start, '(');
    if (!end) return true;
    ffStrbufSetNS((FFstrbuf*) userdata, (uint32_t) (end - start), start);
    return false;
}

static bool getShellVersionBash(FFstrbuf* exe, FFstrbuf* exePath, FFstrbuf* version)
{
    const char* path = exePath->chars;
    if (*path == '\0')
        path = exe->chars;
    ffBinaryExtractStrings(path, extractBashVersion, version, (uint32_t) strlen("@(#)Bash version 0.0.0(0) release GNU"));

    if(!getExeVersionRaw(exe, version))
        return false;

    // GNU bash, version 5.1.16(1)-release (x86_64-pc-msys)\nCopyright...
    ffStrbufSubstrBeforeFirstC(version, '('); // GNU bash, version 5.1.16
    ffStrbufSubstrAfterLastC(version, ' '); // 5.1.16
    return true;
}

static bool getShellVersionFish(FFstrbuf* exe, FFstrbuf* version)
{
    if(!getExeVersionRaw(exe, version))
        return false;

    //fish, version 3.6.0
    ffStrbufSubstrAfterLastC(version, ' ');
    return true;
}

static bool getShellVersionPwsh(FFstrbuf* exe, FFstrbuf* version)
{
    #ifdef _WIN32
    if(getFileVersion(exe->chars, version))
    {
        ffStrbufSubstrBeforeLastC(version, '.');
        return true;
    }
    #endif

    if(!getExeVersionRaw(exe, version))
        return false;

    ffStrbufSubstrAfterLastC(version, ' ');
    return true;
}

static bool getShellVersionKsh(FFstrbuf* exe, FFstrbuf* version)
{
#if __OpenBSD__
    if(ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "-c",
        "echo $KSH_VERSION",
        NULL
    }) != NULL)
        return false;

    // @(#)PD KSH v5.2.14 99/07/13.2
    ffStrbufSubstrAfterFirstC(version, 'v');
    ffStrbufSubstrBeforeFirstC(version, ' ');
#else
    if(ffProcessAppendStdErr(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    }) != NULL)
        return false;

    //  version         sh (AT&T Research) 93u+ 2012-08-01
    ffStrbufSubstrAfterLastC(version, ')');
    ffStrbufTrim(version, ' ');
#endif
    return true;
}

static bool getShellVersionOksh(FFstrbuf* exe, FFstrbuf* version)
{
    if(ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "-c",
        "echo $OKSH_VERSION",
        NULL
    }) != NULL)
        return false;

    //oksh 7.3
    ffStrbufSubstrAfterFirstC(version, ' ');
    return true;
}

static bool getShellVersionOils(FFstrbuf* exe, FFstrbuf* version)
{
    if(ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    }) != NULL)
        return false;

    // Oils 0.18.0		https://www.oilshell.org/...
    ffStrbufSubstrAfterFirstC(version, ' ');
    ffStrbufSubstrBeforeFirstC(version, '\t');
    return true;
}

static bool getShellVersionNushell(FFstrbuf* exe, FFstrbuf* version)
{
    ffStrbufSetS(version, getenv("NU_VERSION"));
    if (version->length) return true;
    return getExeVersionRaw(exe, version); //0.73.0
}

static bool getShellVersionAsh(FFstrbuf* exe, FFstrbuf* version)
{
    if(ffProcessAppendStdErr(version, (char* const[]) {
        exe->chars,
        "--help",
        NULL
    }) != NULL)
        return false;

    // BusyBox v1.36.1 (2023-11-07 18:53:09 UTC) multi-call binary...
    ffStrbufSubstrAfterFirstC(version, ' ');
    ffStrbufSubstrBeforeFirstC(version, ' ');
    ffStrbufTrimLeft(version, 'v');
    return true;
}

static bool getShellVersionXonsh(FFstrbuf* exe, FFstrbuf* version)
{
    ffStrbufSetS(version, getenv("XONSH_VERSION"));
    if (version->length) return true;

    if(ffProcessAppendStdErr(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    }) != NULL)
        return false;

    // xonsh/0.14.1
    ffStrbufSubstrAfterFirstC(version, '/');
    return true;
}

static bool extractZshVersion(const char* line, FF_MAYBE_UNUSED uint32_t len, void *userdata)
{
    if (!ffStrStartsWith(line, "zsh-")) return true;
    const char* start = line + strlen("zsh-");
    const char* end = strchr(start, '-');
    if (!end) return true;

    ffStrbufSetNS((FFstrbuf*) userdata, (uint32_t) (end - start), start);
    return false;
}

static bool getShellVersionZsh(FFstrbuf* exe, FFstrbuf* exePath, FFstrbuf* version)
{
    const char* path = exePath->chars;
    if (*path == '\0')
        path = exe->chars;

    ffBinaryExtractStrings(path, extractZshVersion, version, (uint32_t) strlen("zsh-0.0-0"));
    if (version->length) return true;

    return getExeVersionGeneral(exe, version); //zsh 5.9 (arm-apple-darwin21.3.0)
}

#ifdef _WIN32
static bool getShellVersionWinPowerShell(FFstrbuf* exe, FFstrbuf* version)
{
    return ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "-NoLogo",
        "-NoProfile",
        "-Command",
        "$PSVersionTable.PSVersion.ToString()",
        NULL
    }) == NULL;
}
#endif

bool fftsGetShellVersion(FFstrbuf* exe, const char* exeName, FFstrbuf* exePath, FFstrbuf* version)
{
    if (!instance.config.general.detectVersion) return false;

    if(ffStrEqualsIgnCase(exeName, "sh")) // #849
        return false;

    if(ffStrEqualsIgnCase(exeName, "bash"))
        return getShellVersionBash(exe, exePath, version);
    if(ffStrEqualsIgnCase(exeName, "zsh"))
        return getShellVersionZsh(exe, exePath, version);
    if(ffStrEqualsIgnCase(exeName, "fish"))
        return getShellVersionFish(exe, version);
    if(ffStrEqualsIgnCase(exeName, "pwsh"))
        return getShellVersionPwsh(exe, version);
    if(ffStrEqualsIgnCase(exeName, "csh") || ffStrEqualsIgnCase(exeName, "tcsh"))
        return getExeVersionGeneral(exe, version); //tcsh 6.24.07 (Astron) 2022-12-21 (aarch64-apple-darwin) options wide,nls,dl,al,kan,sm,rh,color,filec
    if(ffStrEqualsIgnCase(exeName, "nu"))
        return getShellVersionNushell(exe, version);
    if(ffStrEqualsIgnCase(exeName, "ksh"))
        return getShellVersionKsh(exe, version);
    if(ffStrEqualsIgnCase(exeName, "oksh"))
        return getShellVersionOksh(exe, version);
    if(ffStrEqualsIgnCase(exeName, "oil.ovm"))
        return getShellVersionOils(exe, version);
    if(ffStrEqualsIgnCase(exeName, "elvish"))
        return getExeVersionRaw(exe, version);
    if(ffStrEqualsIgnCase(exeName, "ash"))
        return getShellVersionAsh(exe, version);
    if(ffStrEqualsIgnCase(exeName, "xonsh"))
        return getShellVersionXonsh(exe, version);

    #ifdef _WIN32
    if(ffStrEqualsIgnCase(exeName, "powershell") || ffStrEqualsIgnCase(exeName, "powershell_ise"))
        return getShellVersionWinPowerShell(exe, version);

    return getFileVersion(exe->chars, version);
    #endif

    return false;
}

FF_MAYBE_UNUSED static bool getTerminalVersionTermux(FFstrbuf* version)
{
    ffStrbufSetS(version, getenv("TERMUX_VERSION"));
    return version->length > 0;
}

static bool extractGeneralVersion(const char *str, FF_MAYBE_UNUSED uint32_t len, void *userdata)
{
    if (!ffCharIsDigit(str[0])) return true;
    int count = 0;
    sscanf(str, "%*d.%*d.%*d%n", &count);
    if (count == 0) return true;
    ffStrbufSetS((FFstrbuf*) userdata, str);
    return false;
}

FF_MAYBE_UNUSED static bool getTerminalVersionGnome(FFstrbuf* exe, FFstrbuf* version)
{
    if (exe->chars[0] == '/')
    {
        ffBinaryExtractStrings(exe->chars, extractGeneralVersion, version, (uint32_t) strlen("0.0.0"));
        if (version->length) return true;
    }

    if(ffProcessAppendStdOut(version, (char* const[]){
        "gnome-terminal",
        "--version",
        NULL
    })) return false;

    //# GNOME Terminal 3.46.7 using VTE 0.70.2 +BIDI +GNUTLS +ICU +SYSTEMD
    ffStrbufSubstrAfterFirstS(version, "Terminal ");
    ffStrbufSubstrBeforeFirstC(version, ' ');
    return true;
}

FF_MAYBE_UNUSED static bool getTerminalVersionXfce4Terminal(FFstrbuf* exe, FFstrbuf* version)
{
    if (exe->chars[0] == '/')
    {
        ffBinaryExtractStrings(exe->chars, extractGeneralVersion, version, (uint32_t) strlen("0.0.0"));
        if (version->length) return true;
    }

    return getExeVersionGeneral(exe, version);//xfce4-terminal 1.0.4 (Xfce 4.18)...
}

FF_MAYBE_UNUSED static bool getTerminalVersionKgx(FFstrbuf* version)
{
    if(ffProcessAppendStdOut(version, (char* const[]){
        "kgx",
        "--version",
        NULL
    })) return false;

    //# KGX 45.0 using VTE 0.74.0 +BIDI +GNUTLS +ICU +SYSTEMD
    ffStrbufSubstrAfterFirstS(version, "KGX ");
    ffStrbufSubstrBeforeFirstC(version, ' ');
    return true;
}

FF_MAYBE_UNUSED static bool getTerminalVersionKonsole(FFstrbuf* exe, FFstrbuf* version)
{
    const char* konsoleVersion = getenv("KONSOLE_VERSION");
    if(konsoleVersion)
    {
        //221201
        long major = strtol(konsoleVersion, NULL, 10);
        if (major >= 0)
        {
            long patch = major % 100;
            major /= 100;
            long minor = major % 100;
            major /= 100;
            ffStrbufSetF(version, "%ld.%ld.%ld", major, minor, patch);
            return true;
        }
    }

    return getExeVersionGeneral(exe, version);
}

FF_MAYBE_UNUSED static bool getTerminalVersionFoot(FFstrbuf* exe, FFstrbuf* version)
{
    uint32_t major = 0, minor = 0, patch = 0;
    if (ffGetTerminalResponse("\e[>c", 3, "\e[>1;%2u%2u%2u;0c", &major, &minor, &patch) == NULL)
    {
        ffStrbufSetF(version, "%u.%u.%u", major, minor, patch);
        return true;
    }

    if(!getExeVersionRaw(exe, version)) return false;

    //foot version: 1.13.1 -pgo +ime -graphemes -assertions
    ffStrbufSubstrAfterFirstS(version, "version: ");
    ffStrbufSubstrBeforeFirstC(version, ' ');
    return true;
}

FF_MAYBE_UNUSED static bool getTerminalVersionMateTerminal(FFstrbuf* exe, FFstrbuf* version)
{
    ffBinaryExtractStrings(exe->chars, extractGeneralVersion, version, (uint32_t) strlen("0.0.0"));
    if (version->length > 0) return true;

    if(!getExeVersionRaw(exe, version)) return false;

    //MATE Terminal 1.26.1
    ffStrbufSubstrAfterLastC(version, ' ');
    return version->length > 0;
}

FF_MAYBE_UNUSED static bool getTerminalVersionCockpit(FFstrbuf* exe, FFstrbuf* version)
{
    if(!getExeVersionRaw(exe, version)) return false;

    //Version: 295\n...
    ffStrbufSubstrBeforeFirstC(version, '\n');
    ffStrbufSubstrAfterFirstC(version, ' ');
    return version->length > 0;
}

FF_MAYBE_UNUSED static bool getTerminalVersionXterm(FFstrbuf* exe, FFstrbuf* version)
{
    ffStrbufSetS(version, getenv("XTERM_VERSION"));
    if (!version->length)
    {
        if(ffProcessAppendStdOut(version, (char* const[]){
            exe->chars,
            "-v",
            NULL
        })) return false;
    }

    //xterm(273)
    ffStrbufTrimRight(version, ')');
    ffStrbufSubstrAfterFirstC(version, '(');
    return version->length > 0;
}

FF_MAYBE_UNUSED static bool getTerminalVersionBlackbox(FFstrbuf* exe, FFstrbuf* version)
{
    if(ffProcessAppendStdOut(version, (char* const[]){
        exe->chars,
        "--version",
        NULL
    })) return false;

    //BlackBox version 0.14.0 (flatpak)
    ffStrbufSubstrAfterFirstS(version, "version ");
    ffStrbufSubstrBeforeFirstC(version, ' ');
    return version->length > 0;
}

FF_MAYBE_UNUSED static bool getTerminalVersionUrxvt(FF_MAYBE_UNUSED FFstrbuf* exe, FFstrbuf* version)
{
    if(ffProcessAppendStdErr(version, (char* const[]){
        "urxvt", // Don't use exe because of urxvtd
        "-invalid",
        NULL
    })) return false;

    //urxvt: "invalid": unknown or malformed option.
    //rxvt-unicode (urxvt) v9.31 - released: 2023-01-02
    ffStrbufSubstrAfterFirstS(version, "(urxvt) v");
    ffStrbufSubstrBeforeFirstC(version, ' ');

    return version->length > 0;
}

FF_MAYBE_UNUSED static bool getTerminalVersionSt(FF_MAYBE_UNUSED FFstrbuf* exe, FFstrbuf* version)
{
    if(ffProcessAppendStdErr(version, (char* const[]){
        exe->chars,
        "-v",
        NULL
    })) return false;

    //st 0.9
    ffStrbufSubstrAfterFirstC(version, ' ');

    return version->length > 0;
}

FF_MAYBE_UNUSED static bool getTerminalVersionLxterminal(FFstrbuf* exe, FFstrbuf* version)
{
    if(!getExeVersionRaw(exe, version)) return false;
    // lxterminal 0.3.2
    ffStrbufSubstrAfterFirstC(version, ' ');
    return version->length > 0;
}

FF_MAYBE_UNUSED static bool getTerminalVersionWeston(FF_MAYBE_UNUSED FFstrbuf* exe, FFstrbuf* version)
{
    // weston-terminal doesn't report a version, use weston version instead
    if(ffProcessAppendStdOut(version, (char* const[]){
        "weston",
        "--version",
        NULL
    })) return false;

    //weston 8.0.0
    ffStrbufSubstrAfterFirstC(version, ' ');

    return version->length > 0;
}

static bool getTerminalVersionContour(FFstrbuf* exe, FFstrbuf* version)
{
    const char* env = getenv("TERMINAL_VERSION_STRING");
    if (env)
    {
        ffStrbufAppendS(version, env);
        return true;
    }
    if(!getExeVersionRaw(exe, version)) return false;
    // Contour Terminal Emulator 0.3.12.262
    ffStrbufSubstrAfterLastC(version, ' ');
    return version->length > 0;
}

static bool getTerminalVersionScreen(FFstrbuf* exe, FFstrbuf* version)
{
    if(!getExeVersionRaw(exe, version)) return false;
    // Screen version 4.09.01 (GNU) 20-Aug-23
    ffStrbufSubstrAfter(version, (uint32_t) strlen("Screen version ") - 1);
    ffStrbufSubstrBeforeFirstC(version, ' ');
    return version->length > 0;
}

static bool getTerminalVersionTmux(FFstrbuf* exe, FFstrbuf* version)
{
    if (ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "-V",
        NULL
    }) != NULL)
        return false;

    // tmux 3.4
    ffStrbufSubstrAfterFirstC(version, ' ');
    return version->length > 0;
}

static bool getTerminalVersionZellij(FFstrbuf* exe, FFstrbuf* version)
{
    if(!getExeVersionRaw(exe, version)) return false;

    // zellij 0.39.2
    ffStrbufSubstrAfterFirstC(version, ' ');
    return version->length > 0;
}

static bool getTerminalVersionZed(FFstrbuf* exe, FFstrbuf* version)
{
    FF_STRBUF_AUTO_DESTROY cli = ffStrbufCreateCopy(exe);
    ffStrbufSubstrBeforeLastC(&cli, '/');
    ffStrbufAppendS(&cli, "/cli"
        #ifdef _WIN32
            ".exe"
        #endif
    );

    if(ffProcessAppendStdOut(version, (char* const[]) {
        cli.chars,
        "--version",
        NULL
    }) != NULL)
        return false;

    // Zed 0.142.6 – /Applications/Zed.app
    ffStrbufSubstrAfterFirstC(version, ' ');
    ffStrbufSubstrBeforeFirstC(version, ' ');
    return true;
}

#ifndef _WIN32
static bool getTerminalVersionKitty(FFstrbuf* exe, FFstrbuf* version)
{
    #if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
    char buffer[1024] = {};
    if (
        #ifdef __linux__
        ffReadFileData(FASTFETCH_TARGET_DIR_USR "/lib64/kitty/kitty/constants.py", ARRAY_SIZE(buffer) - 1, buffer) ||
        ffReadFileData(FASTFETCH_TARGET_DIR_USR "/lib/kitty/kitty/constants.py", ARRAY_SIZE(buffer) - 1, buffer)
        #else
        ffReadFileData(_PATH_LOCALBASE "/share/kitty/kitty/constants.py", ARRAY_SIZE(buffer) - 1, buffer)
        #endif
    )
    {
        // Starts from version 0.17.0
        // https://github.com/kovidgoyal/kitty/blob/master/kitty/constants.py#L25
        const char* p = memmem(buffer, ARRAY_SIZE(buffer) - 1, "version: Version = Version(", strlen("version: Version = Version("));
        if (p)
        {
            p += strlen("version: Version = Version(");
            int major, minor, patch;
            if (sscanf(p, "%d,%d,%d", &major, &minor, &patch) == 3)
            {
                ffStrbufSetF(version, "%d.%d.%d", major, minor, patch);
                return true;
            }
        }
    }
    #endif

    char versionHex[64];
    // https://github.com/fastfetch-cli/fastfetch/discussions/1030#discussioncomment-9845233
    if (ffGetTerminalResponse(
        "\eP+q6b697474792d71756572792d76657273696f6e\e\\", // kitty-query-version
        1,
        "\eP1+r%*[^=]=%63[^\e]\e\\\\", versionHex) == NULL)
    {
        // decode hex string
        for (const char* p = versionHex; p[0] && p[1]; p += 2)
        {
            unsigned value;
            if (sscanf(p, "%2x", &value) == 1)
                ffStrbufAppendC(version, (char) value);
        }
        return true;
    }

    //kitty 0.21.2 created by Kovid Goyal
    return getExeVersionGeneral(exe, version);
}

FF_MAYBE_UNUSED static bool getTerminalVersionPtyxis(FF_MAYBE_UNUSED FFstrbuf* exe, FFstrbuf* version)
{
    if(ffProcessAppendStdOut(version, (char* const[]) {
        "ptyxis",
        "--version",
        NULL
    }) != NULL)
        return false;

    ffStrbufSubstrBeforeFirstC(version, '\n');
    ffStrbufSubstrAfterFirstC(version, ' ');
    return true;
}
#endif

#ifdef _WIN32

static bool getTerminalVersionWindowsTerminal(FFstrbuf* exe, FFstrbuf* version)
{
    FF_STRBUF_AUTO_DESTROY buildInfoPath;
    ffStrbufInitNS(&buildInfoPath, ffStrbufLastIndexC(exe, '\\') + 1, exe->chars);
    ffStrbufAppendS(&buildInfoPath, "BuildInfo.xml");

    if(ffParsePropFile(buildInfoPath.chars, "StoreVersion=\"", version))
    {
        ffStrbufTrimRight(version, '"');
        return true;
    }

    return getFileVersion(exe->chars, version);
}

static bool getTerminalVersionConEmu(FFstrbuf* exe, FFstrbuf* version)
{
    ffStrbufSetS(version, getenv("ConEmuBuild"));

    if(version->length)
        return true;

    return getFileVersion(exe->chars, version);
}

#endif

bool fftsGetTerminalVersion(FFstrbuf* processName, FF_MAYBE_UNUSED FFstrbuf* exe, FFstrbuf* version)
{
    if (!instance.config.general.detectVersion) return false;

    #ifdef __ANDROID__

    if(ffStrbufEqualS(processName, "com.termux"))
        return getTerminalVersionTermux(version);

    #endif

    #if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__sun)

    if(ffStrbufStartsWithIgnCaseS(processName, "gnome-terminal"))
        return getTerminalVersionGnome(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "konsole"))
        return getTerminalVersionKonsole(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "yakuake"))
        return getTerminalVersionKonsole(exe, version); // yakuake shares code with konsole

    if(ffStrbufIgnCaseEqualS(processName, "xfce4-terminal"))
        return getTerminalVersionXfce4Terminal(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "terminator"))
        return getExeVersionGeneral(exe, version);//terminator 2.1.3

    if(ffStrbufIgnCaseEqualS(processName, "deepin-terminal"))
        return getExeVersionGeneral(exe, version);//deepin-terminal 5.4.36

    if(ffStrbufIgnCaseEqualS(processName, "foot"))
        return getTerminalVersionFoot(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "qterminal"))
        return getExeVersionRaw(exe, version); //1.2.0

    if(ffStrbufIgnCaseEqualS(processName, "mate-terminal"))
        return getTerminalVersionMateTerminal(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "cockpit-bridge"))
        return getTerminalVersionCockpit(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "xterm"))
        return getTerminalVersionXterm(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "blackbox"))
        return getTerminalVersionBlackbox(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "st"))
        return getTerminalVersionSt(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "lxterminal"))
        return getTerminalVersionLxterminal(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "weston-terminal"))
        return getTerminalVersionWeston(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "urxvt") ||
        ffStrbufIgnCaseEqualS(processName, "urxvtd") ||
        ffStrbufIgnCaseEqualS(processName, "rxvt") ||
        ffStrbufIgnCaseEqualS(processName, "rxvt-unicode")
    )
        return getTerminalVersionUrxvt(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "ptyxis-agent"))
        return getTerminalVersionPtyxis(exe, version);

    #endif

    #ifdef _WIN32

    if(ffStrbufIgnCaseEqualS(processName, "WindowsTerminal.exe"))
        return getTerminalVersionWindowsTerminal(exe, version);

    if(ffStrbufStartsWithIgnCaseS(processName, "ConEmu"))
        return getTerminalVersionConEmu(exe, version);

    #endif

    #ifndef _WIN32

    if(ffStrbufIgnCaseEqualS(processName, "kitty"))
        return getTerminalVersionKitty(exe, version);

    if (ffStrbufIgnCaseEqualS(processName, "Tabby") && getExeVersionRaw(exe, version))
        return true;

    #endif

    if(ffStrbufStartsWithIgnCaseS(processName, "alacritty"))
        return getExeVersionGeneral(exe, version);

    if(ffStrbufStartsWithIgnCaseS(processName, "contour"))
        return getTerminalVersionContour(exe, version);

    if(ffStrbufStartsWithIgnCaseS(processName, "screen"))
        return getTerminalVersionScreen(exe, version);

    if(ffStrbufStartsWithIgnCaseS(processName, "zellij"))
        return getTerminalVersionZellij(exe, version);

    if(ffStrbufStartsWithIgnCaseS(processName, "zed"))
        return getTerminalVersionZed(exe, version);

    const char* termProgramVersion = getenv("TERM_PROGRAM_VERSION");
    if(termProgramVersion)
    {
        const char* termProgram = getenv("TERM_PROGRAM");
        if(termProgram)
        {
            if(ffStrbufStartsWithIgnCaseS(processName, termProgram) || // processName ends with `.exe` on Windows
                (ffStrEquals(termProgram, "vscode") && ffStrbufStartsWithIgnCaseS(processName, "code")) ||

                #ifdef __APPLE__
                (ffStrEquals(termProgram, "iTerm.app") && ffStrbufStartsWithIgnCaseS(processName, "iTermServer-")) ||
                #elif defined(__linux__)
                (ffStrEquals(termProgram, "WarpTerminal") && ffStrbufEqualS(processName, "warp")) ||
                #endif
                false
            ) {
                ffStrbufSetS(version, termProgramVersion);
                return true;
            }
        }
    }

    termProgramVersion = getenv("LC_TERMINAL_VERSION");
    if(termProgramVersion)
    {
        const char* termProgram = getenv("LC_TERMINAL");
        if(termProgram)
        {
            if(ffStrbufStartsWithIgnCaseS(processName, termProgram) || // processName ends with `.exe` on Windows
                (ffStrEquals(termProgram, "vscode") && ffStrbufStartsWithIgnCaseS(processName, "code")) ||
                (ffStrStartsWith(termProgram, "iTerm") && ffStrbufStartsWithIgnCaseS(processName, "iTermServer-"))
            ) {
                ffStrbufSetS(version, termProgramVersion);
                return true;
            }
        }
    }

    if(ffStrbufStartsWithIgnCaseS(processName, "tmux"))
        return getTerminalVersionTmux(exe, version);

    #ifdef _WIN32

    return getFileVersion(exe->chars, version);

    #else

    return false;

    #endif
}
