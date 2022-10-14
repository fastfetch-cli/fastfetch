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

    ffStrbufClear(&result->shellVersion);
    fftsGetShellVersion(&result->shellExe, result->shellPrettyName.chars, &result->shellVersion);

    if(ffStrbufIgnCaseCompS(&result->shellPrettyName, "pwsh") == 0)
        ffStrbufSetS(&result->shellPrettyName, "PowerShell");
    else if(ffStrbufIgnCaseCompS(&result->shellPrettyName, "powershell") == 0)
        ffStrbufSetS(&result->shellPrettyName, "Windows PowerShell");
    else if(ffStrbufIgnCaseCompS(&result->shellPrettyName, "powershell_ise") == 0)
        ffStrbufSetS(&result->shellPrettyName, "Windows PowerShell ISE");
    else if(ffStrbufIgnCaseCompS(&result->shellPrettyName, "cmd") == 0)
        ffStrbufSetS(&result->shellPrettyName, "Command Prompt");
    else if(ffStrbufIgnCaseCompS(&result->shellPrettyName, "nu") == 0)
        ffStrbufSetS(&result->shellPrettyName, "nushell");
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

    if(pid == 0 || !getProcessInfo(pid, &ppid, &result->terminalProcessName, &result->terminalExe))
        return 0;
    result->terminalExeName = result->terminalExe.chars + ffStrbufLastIndexC(&result->terminalExe, '\\');

    ffStrbufSet(&result->terminalPrettyName, &result->terminalProcessName);
    if(ffStrbufEndsWithIgnCaseS(&result->terminalPrettyName, ".exe"))
        ffStrbufSubstrBefore(&result->terminalPrettyName, result->terminalPrettyName.length - 4);

    if(
        ffStrbufIgnCaseCompS(&result->terminalPrettyName, "pwsh") == 0 ||
        ffStrbufIgnCaseCompS(&result->terminalPrettyName, "cmd") == 0 ||
        ffStrbufIgnCaseCompS(&result->terminalPrettyName, "bash") == 0 ||
        ffStrbufIgnCaseCompS(&result->terminalPrettyName, "zsh") == 0 ||
        ffStrbufIgnCaseCompS(&result->terminalPrettyName, "fish") == 0 ||
        ffStrbufIgnCaseCompS(&result->terminalPrettyName, "nu") == 0 ||
        ffStrbufIgnCaseCompS(&result->terminalPrettyName, "powershell") == 0 ||
        ffStrbufIgnCaseCompS(&result->terminalPrettyName, "powershell_ise") == 0
    ) {
        //We are nested shell
        ffStrbufClear(&result->terminalProcessName);
        ffStrbufClear(&result->terminalPrettyName);
        ffStrbufClear(&result->terminalExe);
        result->terminalExeName = NULL;
        return getTerminalInfo(result, ppid);
    }

    if(ffStrbufIgnCaseCompS(&result->terminalPrettyName, "WindowsTerminal") == 0)
        ffStrbufSetS(&result->terminalPrettyName, "Windows Terminal");
    else if(ffStrbufIgnCaseCompS(&result->terminalPrettyName, "conhost") == 0)
        ffStrbufSetS(&result->terminalPrettyName, "Console Window Host");
    else if(ffStrbufIgnCaseCompS(&result->terminalPrettyName, "explorer") == 0)
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

    const char* term = NULL;

    //SSH
    if(getenv("SSH_CONNECTION") != NULL)
        term = getenv("SSH_TTY");

    //Windows Terminal
    if(!term && (
        getenv("WT_SESSION") != NULL ||
        getenv("WT_PROFILE_ID") != NULL
    )) term = "Windows Terminal";

    //Alacritty
    if(!term && (
        getenv("ALACRITTY_SOCKET") != NULL ||
        getenv("ALACRITTY_LOG") != NULL ||
        getenv("ALACRITTY_WINDOW_ID") != NULL
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
