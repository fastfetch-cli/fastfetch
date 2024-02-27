#include "fastfetch.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "common/properties.h"
#include "util/stringUtils.h"

#include <ctype.h>

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

static bool getShellVersionBash(FFstrbuf* exe, FFstrbuf* version)
{
    if(!getExeVersionRaw(exe, version))
        return false;

    // GNU bash, version 5.1.16(1)-release (x86_64-pc-msys)\nCopyright...
    ffStrbufSubstrBeforeFirstC(version, '\n'); // GNU bash, version 5.1.16(1)-release (x86_64-pc-msys)
    ffStrbufSubstrBeforeLastC(version, ' '); // GNU bash, version 5.1.16(1)-release
    ffStrbufSubstrAfterLastC(version, ' '); // 5.1.16(1)-release
    ffStrbufSubstrBeforeFirstC(version, '('); // 5.1.16
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
    if(ffProcessAppendStdErr(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    }) != NULL)
        return false;

    //  version         sh (AT&T Research) 93u+ 2012-08-01
    ffStrbufSubstrAfterLastC(version, ')');
    ffStrbufTrim(version, ' ');
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
#else
static bool getShellVersionGeneric(FFstrbuf* exe, const char* exeName, FFstrbuf* version)
{
    FF_STRBUF_AUTO_DESTROY command = ffStrbufCreate();
    ffStrbufAppendS(&command, "printf \"%s\" \"$");
    ffStrbufAppendTransformS(&command, exeName, toupper);
    ffStrbufAppendS(&command, "_VERSION\"");

    if (ffProcessAppendStdOut(version, (char* const[]) {
        "env",
        "-i",
        exe->chars,
        "-c",
        command.chars,
        NULL
    }) != NULL)
        return false;

    ffStrbufSubstrBeforeFirstC(version, '(');
    ffStrbufRemoveStrings(version, 2, (const char*[]) { "-release", "release" });
    return true;
}
#endif

bool fftsGetShellVersion(FFstrbuf* exe, const char* exeName, FFstrbuf* version)
{
    if(strcasecmp(exeName, "bash") == 0 || strcasecmp(exeName, "sh") == 0)
        return getShellVersionBash(exe, version);
    if(strcasecmp(exeName, "zsh") == 0)
        return getExeVersionGeneral(exe, version); //zsh 5.9 (arm-apple-darwin21.3.0)
    if(strcasecmp(exeName, "fish") == 0)
        return getShellVersionFish(exe, version);
    if(strcasecmp(exeName, "pwsh") == 0)
        return getShellVersionPwsh(exe, version);
    if(strcasecmp(exeName, "csh") == 0 || strcasecmp(exeName, "tcsh") == 0)
        return getExeVersionGeneral(exe, version); //tcsh 6.24.07 (Astron) 2022-12-21 (aarch64-apple-darwin) options wide,nls,dl,al,kan,sm,rh,color,filec
    if(strcasecmp(exeName, "nu") == 0)
        return getShellVersionNushell(exe, version);
    if(strcasecmp(exeName, "ksh") == 0)
        return getShellVersionKsh(exe, version);
    if(strcasecmp(exeName, "oksh") == 0)
        return getShellVersionOksh(exe, version);
    if(strcasecmp(exeName, "oil.ovm") == 0)
        return getShellVersionOils(exe, version);
    if(strcasecmp(exeName, "elvish") == 0)
        return getExeVersionRaw(exe, version);
    if(strcasecmp(exeName, "ash") == 0)
        return getShellVersionAsh(exe, version);
    if(strcasecmp(exeName, "xonsh") == 0)
        return getShellVersionXonsh(exe, version);

    #ifdef _WIN32
    if(strcasecmp(exeName, "powershell") == 0 || strcasecmp(exeName, "powershell_ise") == 0)
        return getShellVersionWinPowerShell(exe, version);

    return getFileVersion(exe->chars, version);
    #else
    return getShellVersionGeneric(exe, exeName, version);
    #endif
}

FF_MAYBE_UNUSED static bool getTerminalVersionTermux(FFstrbuf* version)
{
    ffStrbufSetS(version, getenv("TERMUX_VERSION"));
    return version->length > 0;
}

FF_MAYBE_UNUSED static bool getTerminalVersionGnome(FFstrbuf* version)
{
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
    if(!getExeVersionRaw(exe, version)) return false;

    //foot version: 1.13.1 -pgo +ime -graphemes -assertions
    ffStrbufSubstrAfterFirstS(version, "version: ");
    ffStrbufSubstrBeforeFirstC(version, ' ');
    return true;
}

FF_MAYBE_UNUSED static bool getTerminalVersionMateTerminal(FFstrbuf* exe, FFstrbuf* version)
{
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
    if(ffProcessAppendStdOut(version, (char* const[]){
        exe->chars,
        "-v",
        NULL
    })) return false;

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
    ffStrbufSubstrAfterFirstC(version, ' ');
    return version->length > 0;
}

static bool getTerminalVersionScreen(FFstrbuf* exe, FFstrbuf* version)
{
    if(!getExeVersionRaw(exe, version)) return false;
    // Screen version 4.09.01 (GNU) 20-Aug-23
    ffStrbufSubstrAfter(version, strlen("Screen version ") - 1);
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
    #ifdef __ANDROID__

    if(ffStrbufEqualS(processName, "com.termux"))
        return getTerminalVersionTermux(version);

    #endif

    #if defined(__linux__) || defined(__FreeBSD__)

    if(ffStrbufStartsWithIgnCaseS(processName, "gnome-terminal"))
        return getTerminalVersionGnome(version);

    if(ffStrbufIgnCaseEqualS(processName, "konsole"))
        return getTerminalVersionKonsole(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "yakuake"))
        return getExeVersionGeneral(exe, version);//yakuake 22.12.3

    if(ffStrbufIgnCaseEqualS(processName, "xfce4-terminal"))
        return getExeVersionGeneral(exe, version);//xfce4-terminal 1.0.4 (Xfce 4.18)...

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

    if(ffStrbufIgnCaseEqualS(processName, "urxvt") ||
        ffStrbufIgnCaseEqualS(processName, "urxvtd") ||
        ffStrbufIgnCaseEqualS(processName, "rxvt") ||
        ffStrbufIgnCaseEqualS(processName, "rxvt-unicode")
    )
        return getTerminalVersionUrxvt(exe, version);

    #endif

    #ifdef _WIN32

    if(ffStrbufIgnCaseEqualS(processName, "WindowsTerminal.exe"))
        return getTerminalVersionWindowsTerminal(exe, version);

    if(ffStrbufStartsWithIgnCaseS(processName, "ConEmu"))
        return getTerminalVersionConEmu(exe, version);

    #endif

    #ifndef _WIN32

    if(ffStrbufIgnCaseEqualS(processName, "kitty"))
        return getExeVersionGeneral(exe, version); //kitty 0.21.2 created by Kovid Goyal

    if (ffStrbufIgnCaseEqualS(processName, "Tabby") && getExeVersionRaw(exe, version))
        return true;

    #endif

    if(ffStrbufStartsWithIgnCaseS(processName, "alacritty"))
        return getExeVersionGeneral(exe, version);

    if(ffStrbufStartsWithIgnCaseS(processName, "contour"))
        return getTerminalVersionContour(exe, version);

    if(ffStrbufStartsWithIgnCaseS(processName, "screen"))
        return getTerminalVersionScreen(exe, version);

    const char* termProgramVersion = getenv("TERM_PROGRAM_VERSION");
    if(termProgramVersion)
    {
        const char* termProgram = getenv("TERM_PROGRAM");
        if(termProgram)
        {
            if(ffStrbufStartsWithIgnCaseS(processName, termProgram) || // processName ends with `.exe` on Windows
                (ffStrEquals(termProgram, "vscode") && ffStrbufStartsWithIgnCaseS(processName, "code")) ||
                (ffStrEquals(termProgram, "iTerm.app") && ffStrbufStartsWithIgnCaseS(processName, "iTermServer-"))
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
