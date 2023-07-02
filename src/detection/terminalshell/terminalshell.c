#include "fastfetch.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "common/properties.h"

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
    bool ok = ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    }) == NULL;
    if (ok)
    {
        ffStrbufTrim(version, '\n');
        ffStrbufTrim(version, ' ');
    }
    return ok;
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
    ffStrbufTrimRight(version, '\n');
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

    ffStrbufTrimRight(version, '\n');
    ffStrbufSubstrAfterLastC(version, ' ');
    return true;
}

#ifdef _WIN32
static bool getShellVersionWinPowerShell(FFstrbuf* exe, FFstrbuf* version)
{
    if(ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "-NoLogo",
        "-NoProfile",
        "-Command",
        "$PSVersionTable.PSVersion.ToString()",
        NULL
    })) return false;

    ffStrbufTrimRight(version, '\n');
    ffStrbufSubstrAfterLastC(version, ' ');
    return true;
}

static bool getShellVersionCmd(FFstrbuf* exe, FFstrbuf* version)
{
    return getFileVersion(exe->chars, version);
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
        return getExeVersionRaw(exe, version); //0.73.0
    if(strcasecmp(exeName, "python") == 0 && getenv("XONSH_VERSION"))
    {
        ffStrbufSetS(version, getenv("XONSH_VERSION"));
        return true;
    }

    #ifdef _WIN32
    if(strcasecmp(exeName, "powershell") == 0 || strcasecmp(exeName, "powershell_ise") == 0)
        return getShellVersionWinPowerShell(exe, version);
    if(strcasecmp(exeName, "cmd") == 0)
        return getShellVersionCmd(exe, version);
    #endif

    return false;
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

    if(ffStrbufEqualS(processName, "Termux"))
        return getTerminalVersionTermux(version);

    #endif

    #if defined(__linux__) || defined(__FreeBSD__)

    if(ffStrbufStartsWithIgnCaseS(processName, "gnome-terminal-"))
        return getTerminalVersionGnome(version);

    if(ffStrbufIgnCaseEqualS(processName, "konsole"))
        return getTerminalVersionKonsole(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "xfce4-terminal"))
        return getExeVersionGeneral(exe, version);//xfce4-terminal 1.0.4 (Xfce 4.18)...

    if(ffStrbufIgnCaseEqualS(processName, "deepin-terminal"))
        return getExeVersionGeneral(exe, version);//deepin-terminal 5.4.36

    if(ffStrbufIgnCaseEqualS(processName, "foot"))
        return getTerminalVersionFoot(exe, version);

    if(ffStrbufIgnCaseEqualS(processName, "qterminal"))
        return getExeVersionRaw(exe, version); //1.2.0

    if(ffStrbufIgnCaseEqualS(processName, "mate-terminal"))
        return getTerminalVersionMateTerminal(exe, version);

    #endif

    #ifdef _WIN32

    if(ffStrbufIgnCaseEqualS(processName, "WindowsTerminal.exe"))
        return getTerminalVersionWindowsTerminal(exe, version);

    if(ffStrbufStartsWithIgnCaseS(processName, "ConEmuC"))
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

    const char* termProgramVersion = getenv("TERM_PROGRAM_VERSION");
    if(termProgramVersion)
    {
        const char* termProgram = getenv("TERM_PROGRAM");
        if(termProgram)
        {
            if(ffStrbufStartsWithIgnCaseS(processName, termProgram) || // processName ends with `.exe` on Windows
                (strcmp(termProgram, "vscode") == 0 && ffStrbufStartsWithIgnCaseS(processName, "code")) ||
                (strcmp(termProgram, "iTerm.app") == 0 && ffStrbufStartsWithIgnCaseS(processName, "iTermServer-"))
            ) {
                ffStrbufSetS(version, termProgramVersion);
                return true;
            }
        }
    }

    #ifdef _WIN32

    return getFileVersion(exe->chars, version);

    #else

    return false;

    #endif
}
