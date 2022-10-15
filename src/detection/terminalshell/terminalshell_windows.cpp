extern "C" {
#include "terminalshell.h"
#include "common/processing.h"
#include "common/thread.h"
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
    swprintf(query, 256, L"SELECT %ls %ls ParentProcessId FROM Win32_Process WHERE ProcessId = %" PRIu32,
        pname ? L"Name," : L"",
        pname ? L"ExecutablePath," : L"",
    pid);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(query, nullptr);
    if(!pEnumerator)
        return false;

    IWbemClassObject *pclsObj = nullptr;
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

extern "C" bool fftsGetShellVersion(FFstrbuf* exe, const char* exeName, FFstrbuf* version);

static uint32_t getShellInfo(FFTerminalShellResult* result, uint32_t pid)
{
    uint32_t ppid;

    if(pid == 0 || !getProcessInfo(pid, &ppid, &result->shellProcessName, &result->shellExe))
        return 0;
    result->shellExeName = result->shellExe.chars + ffStrbufLastIndexC(&result->shellExe, '\\') + 1;

    ffStrbufSet(&result->shellPrettyName, &result->shellProcessName);
    if(ffStrbufEndsWithIgnCaseS(&result->shellPrettyName, ".exe"))
        ffStrbufSubstrBefore(&result->shellPrettyName, result->shellPrettyName.length - 4);

    //Common programs that are between terminal and own process, but are not the shell
    if(
        ffStrbufIgnCaseEqualS(&result->shellPrettyName, "sudo")          ||
        ffStrbufIgnCaseEqualS(&result->shellPrettyName, "su")            ||
        ffStrbufIgnCaseEqualS(&result->shellPrettyName, "doas")          ||
        ffStrbufIgnCaseEqualS(&result->shellPrettyName, "strace")        ||
        ffStrbufIgnCaseEqualS(&result->shellPrettyName, "sshd")          ||
        ffStrbufIgnCaseEqualS(&result->shellPrettyName, "gdb")           ||
        ffStrbufIgnCaseEqualS(&result->shellPrettyName, "lldb")          ||
        ffStrbufIgnCaseEqualS(&result->shellPrettyName, "guake-wrapped") ||
        ffStrbufContainIgnCaseS(&result->shellPrettyName, "debug")
    ) {
        ffStrbufClear(&result->shellProcessName);
        ffStrbufClear(&result->shellPrettyName);
        ffStrbufClear(&result->shellExe);
        result->shellExeName = nullptr;
        return getShellInfo(result, ppid);
    }

    ffStrbufClear(&result->shellVersion);
    fftsGetShellVersion(&result->shellExe, result->shellPrettyName.chars, &result->shellVersion);

    if(ffStrbufIgnCaseEqualS(&result->shellPrettyName, "pwsh"))
        ffStrbufSetS(&result->shellPrettyName, "PowerShell");
    else if(ffStrbufIgnCaseEqualS(&result->shellPrettyName, "powershell"))
        ffStrbufSetS(&result->shellPrettyName, "Windows PowerShell");
    else if(ffStrbufIgnCaseEqualS(&result->shellPrettyName, "powershell_ise"))
        ffStrbufSetS(&result->shellPrettyName, "Windows PowerShell ISE");
    else if(ffStrbufIgnCaseEqualS(&result->shellPrettyName, "cmd"))
        ffStrbufSetS(&result->shellPrettyName, "Command Prompt");
    else if(ffStrbufIgnCaseEqualS(&result->shellPrettyName, "nu"))
        ffStrbufSetS(&result->shellPrettyName, "nushell");
    else if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "explorer"))
    {
        ffStrbufSetS(&result->terminalPrettyName, "Windows Explorer"); // Started without shell
        return 0;
    }

    return ppid;
}

static uint32_t getTerminalInfo(FFTerminalShellResult* result, uint32_t pid)
{
    uint32_t ppid;

    if(pid == 0 || !getProcessInfo(pid, &ppid, &result->terminalProcessName, &result->terminalExe))
        return 0;
    result->terminalExeName = result->terminalExe.chars + ffStrbufLastIndexC(&result->terminalExe, '\\');

    ffStrbufSet(&result->terminalPrettyName, &result->terminalProcessName);
    if(ffStrbufEndsWithIgnCaseS(&result->terminalPrettyName, ".exe"))
        ffStrbufSubstrBefore(&result->terminalPrettyName, result->terminalPrettyName.length - 4);

    if(
        ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "pwsh")           ||
        ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "cmd")            ||
        ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "bash")           ||
        ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "zsh")            ||
        ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "fish")           ||
        ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "nu")             ||
        ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "powershell")     ||
        ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "powershell_ise")
    ) {
        //We are nested shell
        ffStrbufClear(&result->terminalProcessName);
        ffStrbufClear(&result->terminalPrettyName);
        ffStrbufClear(&result->terminalExe);
        result->terminalExeName = nullptr;
        return getTerminalInfo(result, ppid);
    }

    if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "WindowsTerminal"))
        ffStrbufSetS(&result->terminalPrettyName, "Windows Terminal");
    else if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "conhost"))
        ffStrbufSetS(&result->terminalPrettyName, "Console Window Host");
    else if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "Code"))
        ffStrbufSetS(&result->terminalPrettyName, "Visual Studio Code");
    else if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "explorer"))
        ffStrbufSetS(&result->terminalPrettyName, "Windows Explorer");

    return ppid;
}

static void getTerminalFromEnv(FFTerminalShellResult* result)
{
    if(
        result->terminalProcessName.length > 0 &&
        !ffStrbufStartsWithIgnCaseS(&result->terminalProcessName, "login") &&
        ffStrbufIgnCaseCompS(&result->terminalProcessName, "(login)") != 0 &&
        ffStrbufIgnCaseCompS(&result->terminalProcessName, "systemd") != 0 &&
        ffStrbufIgnCaseCompS(&result->terminalProcessName, "init") != 0 &&
        ffStrbufIgnCaseCompS(&result->terminalProcessName, "(init)") != 0 &&
        ffStrbufIgnCaseCompS(&result->terminalProcessName, "0") != 0
    ) return;

    const char* term = nullptr;

    //SSH
    if(getenv("SSH_CONNECTION") != nullptr)
        term = getenv("SSH_TTY");

    //Windows Terminal
    if(!term && (
        getenv("WT_SESSION") != nullptr ||
        getenv("WT_PROFILE_ID") != nullptr
    )) term = "Windows Terminal";

    //Alacritty
    if(!term && (
        getenv("ALACRITTY_SOCKET") != nullptr ||
        getenv("ALACRITTY_LOG") != nullptr ||
        getenv("ALACRITTY_WINDOW_ID") != nullptr
    )) term = "Alacritty";

    //Normal Terminal
    if(!term)
        term = getenv("TERM");

    if(term)
    {
        ffStrbufSetS(&result->terminalProcessName, term);
        ffStrbufSetS(&result->terminalPrettyName, term);
        ffStrbufSetS(&result->terminalExe, term);
        result->terminalExeName = "";
    }
}

const FFTerminalShellResult* ffDetectTerminalShell(const FFinstance* instance)
{
    FF_UNUSED(instance);

    static FFThreadMutex mutex = FF_THREAD_MUTEX_INITIALIZER;
    static FFTerminalShellResult result;
    static bool init = false;
    ffThreadMutexLock(&mutex);
    if(init)
    {
        ffThreadMutexUnlock(&mutex);
        return &result;
    }
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
        goto exit;

    ppid = getShellInfo(&result, ppid);
    getTerminalInfo(&result, ppid);
    if(result.terminalProcessName.length == 0)
        getTerminalFromEnv(&result);

exit:
    ffThreadMutexUnlock(&mutex);
    return &result;
}
