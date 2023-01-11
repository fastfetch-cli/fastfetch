#include "fastfetch.h"
#include "common/processing.h"

#ifdef _WIN32

#include <winver.h>

static bool getFileVersion(const char* exePath, FFstrbuf* version)
{
    DWORD handle;
    DWORD size = GetFileVersionInfoSizeA(exePath, &handle);
    if(size > 0)
    {
        void* versionData = malloc(size);
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
                free(versionData);
                return true;
            }
        }
        free(versionData);
    }

    return false;
}

#endif

static void getShellVersionBash(FFstrbuf* exe, FFstrbuf* version)
{
    ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    }); // GNU bash, version 5.1.16(1)-release (x86_64-pc-msys)\nCopyright...
    ffStrbufSubstrBeforeFirstC(version, '\n'); // GNU bash, version 5.1.16(1)-release (x86_64-pc-msys)
    ffStrbufSubstrBeforeLastC(version, ' '); // GNU bash, version 5.1.16(1)-release
    ffStrbufSubstrAfterLastC(version, ' '); // 5.1.16(1)-release
    ffStrbufSubstrBeforeFirstC(version, '('); // 5.1.16
}

static void getShellVersionZsh(FFstrbuf* exe, FFstrbuf* version)
{
    ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    });
    ffStrbufSubstrBeforeLastC(version, ' ');
    ffStrbufSubstrAfterFirstC(version, ' ');
}

static void getShellVersionFish(FFstrbuf* exe, FFstrbuf* version)
{
    ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    });
    ffStrbufTrimRight(version, '\n');
    ffStrbufSubstrAfterLastC(version, ' ');
}

static void getShellVersionTcsh(FFstrbuf* exe, FFstrbuf* version)
{
    ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    }); // tcsh 6.24.01 (Astron) 2022-05-12 (aarch64-apple-darwin) options wide,nls,dl,al,kan,sm,rh,color,filec
    ffStrbufSubstrAfterFirstC(version, ' '); // 6.24.01 (Astron) 2022-05-12 (aarch64-apple-darwin) options wide,nls,dl,al,kan,sm,rh,color,filec
    ffStrbufSubstrBeforeFirstC(version, ' '); // 6.24.01
}

static void getShellVersionPwsh(FFstrbuf* exe, FFstrbuf* version)
{
    #ifdef _WIN32
    if(getFileVersion(exe->chars, version))
        return;
    #endif

    ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    });
    ffStrbufTrimRight(version, '\n');
    ffStrbufSubstrAfterLastC(version, ' ');
}

#ifdef _WIN32
static void getShellVersionWinPowerShell(FFstrbuf* exe, FFstrbuf* version)
{
    ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "-NoLogo",
        "-NoProfile",
        "-Command",
        "$PSVersionTable.PSVersion.ToString()",
        NULL
    });
    ffStrbufTrimRight(version, '\n');
    ffStrbufSubstrAfterLastC(version, ' ');
}

static void getShellVersionCmd(FFstrbuf* exe, FFstrbuf* version)
{
    getFileVersion(exe->chars, version);
}
#endif

static void getShellVersionNu(FFstrbuf* exe, FFstrbuf* version)
{
    ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    });
}

bool fftsGetShellVersion(FFstrbuf* exe, const char* exeName, FFstrbuf* version)
{
    bool ok = true;
    if(strcasecmp(exeName, "bash") == 0 || strcasecmp(exeName, "sh") == 0)
        getShellVersionBash(exe, version);
    else if(strcasecmp(exeName, "zsh") == 0)
        getShellVersionZsh(exe, version);
    else if(strcasecmp(exeName, "fish") == 0)
        getShellVersionFish(exe, version);
    else if(strcasecmp(exeName, "pwsh") == 0)
        getShellVersionPwsh(exe, version);
    else if(strcasecmp(exeName, "csh") == 0 || strcasecmp(exeName, "tcsh") == 0)
        getShellVersionTcsh(exe, version);
    else if(strcasecmp(exeName, "nu") == 0)
        getShellVersionNu(exe, version);

    #ifdef _WIN32
    else if(strcasecmp(exeName, "powershell") == 0 || strcasecmp(exeName, "powershell_ise") == 0)
        getShellVersionWinPowerShell(exe, version);
    else if(strcasecmp(exeName, "cmd") == 0)
        getShellVersionCmd(exe, version);
    #endif

    else
        ok = false;
    return ok;
}

FF_MAYBE_UNUSED static bool getTerminalVersionTermux(FFstrbuf* version)
{
    ffStrbufSetS(version, getenv("TERMUX_VERSION"));
    return true;
}

bool fftsGetTerminalVersion(FFstrbuf* processName, FF_MAYBE_UNUSED FFstrbuf* exe, FFstrbuf* version)
{
    #ifdef __ANDROID__

    if(ffStrbufEqualS(processName, "Termux"))
        return getTerminalVersionTermux(version);

    #endif

    const char* termProgramVersion = getenv("TERM_PROGRAM_VERSION");
    if(termProgramVersion)
    {
        const char* termProgram = getenv("TERM_PROGRAM");
        if(termProgram)
        {
            if(ffStrbufStartsWithIgnCaseS(processName, termProgram) || // processName ends with `.exe` on Windows
                (strcmp(termProgram, "vscode") == 0 && ffStrbufStartsWithIgnCaseS(processName, "code"))
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
