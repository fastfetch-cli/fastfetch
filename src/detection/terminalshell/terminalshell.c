#include "fastfetch.h"
#include "common/processing.h"

static void getShellVersionBash(FFstrbuf* exe, FFstrbuf* version)
{
    ffProcessAppendStdOut(version, (char* const[]) {
        "env",
        "-i",
        exe->chars,
        "--norc",
        "--noprofile",
        "-c",
        "printf \"%s\" \"$BASH_VERSION\"",
        NULL
    });
    ffStrbufSubstrBeforeFirstC(version, '(');
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

static void getShellVersionFishPwsh(FFstrbuf* exe, FFstrbuf* version)
{
    ffProcessAppendStdOut(version, (char* const[]) {
        exe->chars,
        "--version",
        NULL
    });
    ffStrbufTrimRight(version, '\n');
    ffStrbufSubstrAfterLastC(version, ' ');
}

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
    else if(strcasecmp(exeName, "fish") == 0 || strcasecmp(exeName, "pwsh") == 0)
        getShellVersionFishPwsh(exe, version);
    else if(strcasecmp(exeName, "nu") == 0)
        getShellVersionNu(exe, version);
    else
        ok = false;
    return ok;
}
