extern "C" {
#include "terminalshell.h"
#include "common/processing.h"
}
#include "util/windows/wmi.hpp"

#include <inttypes.h>
#include <processthreadsapi.h>
#include <wchar.h>

struct ProcessInfo
{
    uint32_t pid;
    FFstrbuf psName;
};

static bool getProcessInfo(uint32_t pid, uint32_t* ppid, FFstrbuf* pname, FFstrbuf* exe)
{
    wchar_t query[256] = {};
    swprintf(query, 256, L"SELECT Name, ParentProcessId, ExecutablePath FROM Win32_Process WHERE ProcessId = %" PRIu32, pid);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(query, nullptr);
    if(!pEnumerator)
        return false;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    if(FAILED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) || uReturn == 0)
    {
        pEnumerator->Release();
        return false;
    }

    if(ppid)
    {
        uint64_t value;
        ffGetWmiObjUnsigned(pclsObj, L"ParentProcessId", &value);
        *ppid = (uint32_t) value;
    }

    if(pname)
        ffGetWmiObjString(pclsObj, L"Name", pname);

    if(exe)
        ffGetWmiObjString(pclsObj, L"ExecutablePath", exe);

    pclsObj->Release();
    pEnumerator->Release();
    return true;
}

static void getShellVersion(FFstrbuf* exe, FFstrbuf* version)
{
    char* const argv[] = { exe->chars, (char*)"--version", NULL };
    ffProcessAppendStdOut(version, argv);
    ffStrbufTrimRight(version, '\n');
    ffStrbufSubstrAfterLastC(version, ' ');
}

static uint32_t getShellInfo(FFTerminalShellResult* result, uint32_t pid)
{
    uint32_t ppid;

    if(!getProcessInfo(pid, &ppid, &result->shellProcessName, &result->shellExe))
        return 0;
    result->shellExeName = result->shellExe.chars + ffStrbufLastIndexC(&result->shellExe, '\\') + 1;

    ffStrbufSet(&result->shellPrettyName, &result->shellProcessName);
    if(ffStrbufEndsWithIgnCaseS(&result->shellPrettyName, ".exe"))
        ffStrbufSubstrBefore(&result->shellPrettyName, result->shellPrettyName.length - 4);

    if(ffStrbufIgnCaseCompS(&result->shellPrettyName, "pwsh") == 0)
    {
        ffStrbufSetS(&result->shellPrettyName, "PowerShell");
        getShellVersion(&result->shellExe, &result->shellVersion);
    }
    else if(ffStrbufIgnCaseCompS(&result->shellPrettyName, "powershell") == 0)
        ffStrbufSetS(&result->shellPrettyName, "Windows PowerShell");
    else if(ffStrbufIgnCaseCompS(&result->shellPrettyName, "powershell_ise") == 0)
        ffStrbufSetS(&result->shellPrettyName, "Windows PowerShell ISE");
    else if(ffStrbufIgnCaseCompS(&result->shellPrettyName, "cmd") == 0)
        ffStrbufSetS(&result->shellPrettyName, "Command Prompt");
    else if(ffStrbufIgnCaseCompS(&result->terminalPrettyName, "explorer") == 0)
    {
        ffStrbufSetS(&result->terminalPrettyName, "Windows Explorer"); // Started without shell
        return 0;
    }

    return ppid;
}

static uint32_t getTerminalInfo(FFTerminalShellResult* result, uint32_t pid)
{
    uint32_t ppid;

    if(!getProcessInfo(pid, &ppid, &result->terminalProcessName, &result->terminalExe))
        return 0;
    result->terminalExeName = result->terminalExe.chars + ffStrbufLastIndexC(&result->terminalExe, '\\');

    ffStrbufSet(&result->terminalPrettyName, &result->terminalProcessName);
    if(ffStrbufEndsWithIgnCaseS(&result->terminalPrettyName, ".exe"))
        ffStrbufSubstrBefore(&result->terminalPrettyName, result->terminalPrettyName.length - 4);

    if(ffStrbufIgnCaseCompS(&result->terminalPrettyName, "WindowsTerminal") == 0)
        ffStrbufSetS(&result->terminalPrettyName, "Windows Terminal");
    else if(ffStrbufIgnCaseCompS(&result->terminalPrettyName, "conhost") == 0)
        ffStrbufSetS(&result->terminalPrettyName, "Console Window Host");
    else if(ffStrbufIgnCaseCompS(&result->terminalPrettyName, "explorer") == 0)
        ffStrbufSetS(&result->terminalPrettyName, "Windows Explorer");

    return ppid;
}

#ifdef __MSYS__
    extern "C"
    const FFTerminalShellResult* ffDetectTerminalShellPosix(const FFinstance* instance);
#endif

const FFTerminalShellResult* ffDetectTerminalShell(const FFinstance* instance)
{
    #ifdef __MSYS__
        // This is hacky.
        // When running inside of MSYS2, the real Windows parent process doesn't exist and we must find it in Linux way ( /proc/self/xxx )
        // When running outside of MSYS2, /proc/self/xxx doesn't exist and we must find it in Windows way
        if(getenv("MSYSTEM"))
            return ffDetectTerminalShellPosix(instance);
    #else
        FF_UNUSED(instance);
    #endif

    static FFTerminalShellResult result;
    static bool init = false;
    if(init)
        return &result;
    init = true;

    ffStrbufInit(&result.shellProcessName);
    ffStrbufInitA(&result.shellExe, 128);
    result.shellExeName = result.shellExe.chars;
    ffStrbufInit(&result.shellPrettyName);
    ffStrbufInit(&result.shellVersion);

    ffStrbufInit(&result.terminalProcessName);
    ffStrbufInitA(&result.terminalExe, 128);
    result.terminalExeName = result.terminalExe.chars;
    ffStrbufInit(&result.terminalPrettyName);

    ffStrbufInit(&result.userShellExe);
    result.userShellExeName = result.userShellExe.chars;
    ffStrbufInit(&result.userShellVersion);

    uint32_t ppid = GetCurrentProcessId();
    if(!getProcessInfo(ppid, &ppid, nullptr, nullptr))
        return &result;

    ppid = getShellInfo(&result, ppid);
    if(ppid == 0)
        return &result;

    // TODO: handle nested shells

    ppid = getTerminalInfo(&result, ppid);
    if(ppid == 0)
        return &result;

    return &result;
}
