extern "C" {
#include "terminalshell.h"
#include "common/processing.h"
#include "common/thread.h"
}

#include <processthreadsapi.h>
#include <wchar.h>
#include <tlhelp32.h>

#ifdef FF_USE_WIN_NTAPI

#include <winternl.h>

static bool getProcessInfo(uint32_t pid, uint32_t* ppid, FFstrbuf* pname, FFstrbuf* exe, const char** exeName)
{
    HANDLE hProcess = pid == 0
        ? GetCurrentProcess()
        : OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, TRUE, pid);

    if(ppid)
    {
        PROCESS_BASIC_INFORMATION info;
        ULONG size;
        if(NT_SUCCESS(NtQueryInformationProcess(hProcess, ProcessBasicInformation, &info, sizeof(info), &size)))
        {
            assert(size == sizeof(info));
            *ppid = (uint32_t)info.InheritedFromUniqueProcessId;
        }
        else
        {
            CloseHandle(hProcess);
            return false;
        }
    }
    if(exe)
    {
        DWORD bufSize = exe->allocated;
        if(QueryFullProcessImageNameA(hProcess, 0, exe->chars, &bufSize))
            exe->length = bufSize;
        else
        {
            CloseHandle(hProcess);
            return false;
        }
    }
    if(pname && exeName)
    {
        *exeName = exe->chars + ffStrbufLastIndexC(exe, '\\') + 1;
        ffStrbufSetS(pname, *exeName);
    }

    CloseHandle(hProcess);
    return true;
}

#else

#include "util/windows/wmi.hpp"
#include <inttypes.h>

static bool getProcessInfo(uint32_t pid, uint32_t* ppid, FFstrbuf* pname, FFstrbuf* exe, const char** exeName)
{
    if(pid == 0)
        pid = GetCurrentProcessId();

    wchar_t sql[256] = {};
    swprintf(sql, 256, L"SELECT %ls %ls ParentProcessId FROM Win32_Process WHERE ProcessId = %" PRIu32,
        pname ? L"Name," : L"",
        pname ? L"ExecutablePath," : L"",
    pid);

    FFWmiQuery query(sql);
    if(!query)
        return false;

    if(FFWmiRecord record = query.next())
    {
        if(ppid)
        {
            uint64_t value;
            record.getUnsigned(L"ParentProcessId", &value);
            *ppid = (uint32_t) value;
        }

        if(pname)
            record.getString(L"Name", pname);

        if(exe)
            record.getString(L"ExecutablePath", exe);

        if(exeName)
            *exeName = exe->chars + ffStrbufLastIndexC(exe, '\\') + 1;
    }
    return true;
}

#endif

extern "C" bool fftsGetShellVersion(FFstrbuf* exe, const char* exeName, FFstrbuf* version);

static uint32_t getShellInfo(FFTerminalShellResult* result, uint32_t pid)
{
    uint32_t ppid;

    if(pid == 0 || !getProcessInfo(pid, &ppid, &result->shellProcessName, &result->shellExe, &result->shellExeName))
        return 0;

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
    {
        ffStrbufClear(&result->shellPrettyName);

        HANDLE snapshot;
        while(!(snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid)) && GetLastError() == ERROR_BAD_LENGTH) {}

        if(snapshot)
        {
            MODULEENTRY32W module;
            module.dwSize = sizeof(module);
            for(BOOL success = Module32FirstW(snapshot, &module); success; success = Module32NextW(snapshot, &module))
            {
                if(wcsncmp(module.szModule, L"clink_dll_", wcslen(L"clink_dll_")) == 0)
                {
                    ffStrbufAppendS(&result->shellPrettyName, "CMD (with Clink)");
                    break;
                }
            }
            CloseHandle(snapshot);
        }
        if(result->shellPrettyName.length == 0)
            ffStrbufAppendS(&result->shellPrettyName, "Command Prompt");
    }
    else if(ffStrbufIgnCaseEqualS(&result->shellPrettyName, "nu"))
        ffStrbufSetS(&result->shellPrettyName, "nushell");
    else if(ffStrbufIgnCaseEqualS(&result->shellPrettyName, "explorer"))
    {
        ffStrbufSetS(&result->shellPrettyName, "Windows Explorer"); // Started without shell
        return 0;
    }

    return ppid;
}

static uint32_t getTerminalInfo(FFTerminalShellResult* result, uint32_t pid)
{
    uint32_t ppid;

    if(pid == 0 || !getProcessInfo(pid, &ppid, &result->terminalProcessName, &result->terminalExe, &result->terminalExeName))
        return 0;

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
        result->terminalExeName = "";
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
    else if(ffStrbufStartsWithIgnCaseS(&result->terminalPrettyName, "ConEmuC"))
        ffStrbufSetS(&result->terminalPrettyName, "ConEmu");

    return ppid;
}

static void getTerminalFromEnv(FFTerminalShellResult* result)
{
    if(
        result->terminalProcessName.length > 0 &&
        ffStrbufIgnCaseCompS(&result->terminalProcessName, "explorer") != 0
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
    result.shellExeName = "";
    ffStrbufInit(&result.shellPrettyName);
    ffStrbufInit(&result.shellVersion);

    ffStrbufInit(&result.terminalProcessName);
    ffStrbufInitA(&result.terminalExe, 128);
    result.terminalExeName = "";
    ffStrbufInit(&result.terminalPrettyName);

    ffStrbufInit(&result.userShellExe);
    result.userShellExeName = "";
    ffStrbufInit(&result.userShellVersion);

    uint32_t ppid;
    if(!getProcessInfo(0, &ppid, nullptr, nullptr, nullptr))
        goto exit;

    ppid = getShellInfo(&result, ppid);
    getTerminalInfo(&result, ppid);
    if(result.terminalProcessName.length == 0)
        getTerminalFromEnv(&result);

exit:
    ffThreadMutexUnlock(&mutex);
    return &result;
}
