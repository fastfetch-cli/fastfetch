#include "terminalshell.h"
#include "common/processing.h"
#include "common/thread.h"
#include "util/mallocHelper.h"

#include <windows.h>
#include <wchar.h>
#include <tlhelp32.h>
#include <ntstatus.h>
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

static bool getTerminalInfoByEnumeratingChildProcesses(FFTerminalShellResult* result, uint32_t ppid)
{
    ULONG size = 0;
    if(NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &size) != STATUS_INFO_LENGTH_MISMATCH)
        return false;

    size += sizeof(SystemProcessInformation) * 5; //What if new processes are created during two syscalls?

    SYSTEM_PROCESS_INFORMATION* FF_AUTO_FREE pstart = (SYSTEM_PROCESS_INFORMATION*)malloc(size);
    if(!pstart)
        return false;

    if(!NT_SUCCESS(NtQuerySystemInformation(SystemProcessInformation, pstart, size, NULL)))
        return false;

    uint32_t currentProcessId = (uint32_t) GetCurrentProcessId();

    for (SYSTEM_PROCESS_INFORMATION* ptr = pstart; ptr->NextEntryOffset; ptr = (SYSTEM_PROCESS_INFORMATION*)((uint8_t*)ptr + ptr->NextEntryOffset))
    {
        if ((uint32_t)(uintptr_t) ptr->InheritedFromUniqueProcessId != ppid)
            continue;

        uint32_t pid = (uint32_t)(uintptr_t) ptr->UniqueProcessId;
        if (pid == currentProcessId)
            continue;

        if(!getProcessInfo(pid, NULL, &result->terminalProcessName, &result->terminalExe, &result->terminalExeName))
            return false;

        result->terminalPid = pid;
        ffStrbufSet(&result->terminalPrettyName, &result->terminalProcessName);
        if(ffStrbufEndsWithIgnCaseS(&result->terminalPrettyName, ".exe"))
            ffStrbufSubstrBefore(&result->terminalPrettyName, result->terminalPrettyName.length - 4);

        return true;
    }
    return false;
}

bool fftsGetShellVersion(FFstrbuf* exe, const char* exeName, FFstrbuf* version);

static uint32_t getShellInfo(const FFinstance* instance, FFTerminalShellResult* result, uint32_t pid)
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
        ffStrbufIgnCaseEqualS(&result->shellPrettyName, "fastfetch")     || //scoop warps the real binaries with a "shim" exe
        ffStrbufIgnCaseEqualS(&result->shellPrettyName, "flashfetch")    ||
        ffStrbufContainIgnCaseS(&result->shellPrettyName, "debug")
    ) {
        ffStrbufClear(&result->shellProcessName);
        ffStrbufClear(&result->shellPrettyName);
        ffStrbufClear(&result->shellExe);
        result->shellExeName = NULL;
        return getShellInfo(instance, result, ppid);
    }

    ffStrbufClear(&result->shellVersion);
    if(instance->config.shellVersion)
        fftsGetShellVersion(&result->shellExe, result->shellPrettyName.chars, &result->shellVersion);

    result->shellPid = pid;
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
                if(wcsncmp(module.szModule, L"clink_dll_", strlen("clink_dll_")) == 0)
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
        // In this case, terminal process will be created by fastfetch itself.
        return 0;
    }

    return ppid;
}

static uint32_t getTerminalInfo(const FFinstance* instance, FFTerminalShellResult* result, uint32_t pid)
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
        return getTerminalInfo(instance, result, ppid);
    }

    if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "sihost")           ||
        ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "explorer")
    ) {
        ffStrbufClear(&result->terminalProcessName);
        ffStrbufClear(&result->terminalPrettyName);
        ffStrbufClear(&result->terminalExe);
        result->terminalExeName = "";

        // Maybe terminal process is created by shell
        if(!getTerminalInfoByEnumeratingChildProcesses(result, result->shellPid))
            return 0;
    }
    else
        result->terminalPid = pid;

    if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "WindowsTerminal"))
        ffStrbufSetS(&result->terminalPrettyName, ffStrbufContainIgnCaseS(&result->terminalExe, ".WindowsTerminalPreview_")
            ? "Windows Terminal Preview"
            : "Windows Terminal"
        );
    else if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "conhost"))
        ffStrbufSetS(&result->terminalPrettyName, "Console Window Host");
    else if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "Code"))
        ffStrbufSetS(&result->terminalPrettyName, "Visual Studio Code");
    else if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "explorer"))
        ffStrbufSetS(&result->terminalPrettyName, "Windows Explorer");
    else if(ffStrbufStartsWithIgnCaseS(&result->terminalPrettyName, "ConEmuC"))
        ffStrbufSetS(&result->terminalPrettyName, "ConEmu");
    else if(ffStrbufEqualS(&result->terminalPrettyName, "wezterm-gui"))
        ffStrbufInitS(&result->terminalPrettyName, "WezTerm");

    return ppid;
}

static void getTerminalFromEnv(FFTerminalShellResult* result)
{
    if(
        result->terminalProcessName.length > 0 &&
        ffStrbufIgnCaseCompS(&result->terminalProcessName, "explorer") != 0
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

    if(!term)
        term = getenv("TERM_PROGRAM");

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

bool fftsGetTerminalVersion(FFstrbuf* processName, FFstrbuf* exe, FFstrbuf* version);

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
    result.shellPid = 0;

    ffStrbufInit(&result.terminalProcessName);
    ffStrbufInitA(&result.terminalExe, 128);
    result.terminalExeName = "";
    ffStrbufInit(&result.terminalPrettyName);
    result.terminalPid = 0;

    ffStrbufInit(&result.userShellExe);
    result.userShellExeName = "";
    ffStrbufInit(&result.userShellVersion);

    uint32_t ppid;
    if(!getProcessInfo(0, &ppid, NULL, NULL, NULL))
        goto exit;

    ppid = getShellInfo(instance, &result, ppid);
    if(ppid)
        getTerminalInfo(instance, &result, ppid);

    if(result.terminalProcessName.length == 0)
        getTerminalFromEnv(&result);

    ffStrbufInit(&result.terminalVersion);
    if(instance->config.terminalVersion)
        fftsGetTerminalVersion(&result.terminalProcessName, &result.terminalExe, &result.terminalVersion);

exit:
    ffThreadMutexUnlock(&mutex);
    return &result;
}
