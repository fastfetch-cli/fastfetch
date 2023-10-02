#include "terminalshell.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "common/thread.h"
#include "util/mallocHelper.h"
#include "util/windows/registry.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <wchar.h>
#include <tlhelp32.h>
#include <ntstatus.h>
#include <winternl.h>

static bool getProductVersion(const wchar_t* filePath, FFstrbuf* version)
{
    DWORD handle;
    DWORD size = GetFileVersionInfoSizeW(filePath, &handle);
    if(size > 0)
    {
        FF_AUTO_FREE void* versionData = malloc(size);
        if(GetFileVersionInfoW(filePath, handle, size, versionData))
        {
            VS_FIXEDFILEINFO* verInfo;
            UINT len;
            if(VerQueryValueW(versionData, L"\\", (void**)&verInfo, &len) && len && verInfo->dwSignature == 0xFEEF04BD)
            {
                ffStrbufAppendF(version, "%u.%u.%u.%u",
                    (unsigned)(( verInfo->dwProductVersionMS >> 16 ) & 0xffff),
                    (unsigned)(( verInfo->dwProductVersionMS >>  0 ) & 0xffff),
                    (unsigned)(( verInfo->dwProductVersionLS >> 16 ) & 0xffff),
                    (unsigned)(( verInfo->dwProductVersionLS >>  0 ) & 0xffff)
                );
                return true;
            }
        }
    }

    return false;
}

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

bool fftsGetShellVersion(FFstrbuf* exe, const char* exeName, FFstrbuf* version);

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
        ffStrbufIgnCaseEqualS(&result->shellPrettyName, "fastfetch")     || //scoop warps the real binaries with a "shim" exe
        ffStrbufIgnCaseEqualS(&result->shellPrettyName, "flashfetch")    ||
        ffStrbufContainIgnCaseS(&result->shellPrettyName, "debug")       ||
        ffStrbufStartsWithIgnCaseS(&result->shellPrettyName, "ConEmu") // https://github.com/fastfetch-cli/fastfetch/issues/488#issuecomment-1619982014
    ) {
        ffStrbufClear(&result->shellProcessName);
        ffStrbufClear(&result->shellPrettyName);
        ffStrbufClear(&result->shellExe);
        result->shellExeName = NULL;
        return getShellInfo(result, ppid);
    }

    ffStrbufClear(&result->shellVersion);
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
                    ffStrbufAppendS(&result->shellPrettyName, "CMD (with Clink ");
                    getProductVersion(module.szExePath, &result->shellPrettyName);
                    ffStrbufAppendC(&result->shellPrettyName, ')');
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

static bool getTerminalFromEnv(FFTerminalShellResult* result)
{
    if(
        result->terminalProcessName.length > 0 &&
        ffStrbufIgnCaseCompS(&result->terminalProcessName, "explorer") != 0
    ) return false;

    const char* term = getenv("ConEmuPID");

    if(term)
    {
        //ConEmu
        uint32_t pid = (uint32_t) strtoul(term, NULL, 10);
        result->terminalPid = pid;
        if(getProcessInfo(pid, NULL, &result->terminalProcessName, &result->terminalExe, &result->terminalExeName))
        {
            ffStrbufSet(&result->terminalPrettyName, &result->terminalProcessName);
            if(ffStrbufEndsWithIgnCaseS(&result->terminalPrettyName, ".exe"))
                ffStrbufSubstrBefore(&result->terminalPrettyName, result->terminalPrettyName.length - 4);
            return true;
        }
        else
        {
            term = "ConEmu";
        }
    }

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
        return true;
    }

    return false;
}

static bool detectDefaultTerminal(FFTerminalShellResult* result)
{
    wchar_t regPath[128] = L"SOFTWARE\\Classes\\PackagedCom\\ClassIndex\\";
    wchar_t* uuid = regPath + strlen("SOFTWARE\\Classes\\PackagedCom\\ClassIndex\\");
    DWORD bufSize = 80;
    if (RegGetValueW(HKEY_CURRENT_USER, L"Console\\%%Startup", L"DelegationTerminal", RRF_RT_REG_SZ, NULL, uuid, &bufSize) == ERROR_SUCCESS)
    {
        if(wcscmp(uuid, L"{00000000-0000-0000-0000-000000000000}") == 0)
        {
            // Let Windows deside
            return false;
        }
        if(wcscmp(uuid, L"{B23D10C0-E52E-411E-9D5B-C09FDF709C7D}") == 0)
        {
            goto conhost;
        }

        FF_HKEY_AUTO_DESTROY hKey = NULL;
        if(ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regPath, &hKey, NULL))
        {
            FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
            if(ffRegGetSubKey(hKey, 0, &path, NULL))
            {
                if (ffStrbufStartsWithS(&path, "Microsoft.WindowsTerminal"))
                {
                    ffStrbufSetS(&result->terminalProcessName, "WindowsTerminal.exe");
                    ffStrbufSetS(&result->terminalPrettyName, "WindowsTerminal");
                    ffStrbufSetF(&result->terminalExe, "%s\\WindowsApps\\%s\\WindowsTerminal.exe", getenv("ProgramFiles"), path.chars);
                    if(ffPathExists(result->terminalExe.chars, FF_PATHTYPE_FILE))
                    {
                        result->terminalExeName = result->terminalExe.chars + ffStrbufLastIndexC(&result->terminalExe, '\\') + 1;
                    }
                    else
                    {
                        ffStrbufDestroy(&result->terminalExe);
                        ffStrbufInitMove(&result->terminalExe, &path);
                        result->terminalExeName = "";
                    }
                    return true;
                }
            }
        }
    }

conhost:
    ffStrbufSetF(&result->terminalExe, "%s\\System32\\conhost.exe", getenv("SystemRoot"));
    if(ffPathExists(result->terminalExe.chars, FF_PATHTYPE_FILE))
    {
        ffStrbufSetS(&result->terminalProcessName, "conhost.exe");
        ffStrbufSetS(&result->terminalPrettyName, "conhost");
        result->terminalExeName = result->terminalExe.chars + ffStrbufLastIndexC(&result->terminalExe, '\\') + 1;
        return true;
    }

    ffStrbufClear(&result->terminalExe);
    return false;
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
        ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "powershell_ise") ||
        ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "wsl")            || // running inside wsl
        ffStrbufStartsWithIgnCaseS(&result->terminalPrettyName, "ConEmuC") // wrapper process of ConEmu
    ) {
        //We are nested shell
        ffStrbufClear(&result->terminalProcessName);
        ffStrbufClear(&result->terminalPrettyName);
        ffStrbufClear(&result->terminalExe);
        result->terminalExeName = "";
        return getTerminalInfo(result, ppid);
    }

    if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "sihost")           ||
        ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "explorer")
    ) {
        // A CUI program created by Windows Explorer will spawn a conhost as its child.
        // However the conhost process is just a placeholder;
        // The true terminal can be Windows Terminal or others.
        if (!getTerminalFromEnv(result) && !detectDefaultTerminal(result))
        {
            ffStrbufClear(&result->terminalProcessName);
            ffStrbufClear(&result->terminalPrettyName);
            ffStrbufClear(&result->terminalExe);
            result->terminalExeName = "";
            return 0;
        }
    }
    else
        result->terminalPid = pid;

    if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "WindowsTerminal"))
        ffStrbufSetStatic(&result->terminalPrettyName, ffStrbufContainIgnCaseS(&result->terminalExe, ".WindowsTerminalPreview_")
            ? "Windows Terminal Preview"
            : "Windows Terminal"
        );
    else if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "conhost"))
        ffStrbufSetStatic(&result->terminalPrettyName, "Console Window Host");
    else if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "Code"))
        ffStrbufSetStatic(&result->terminalPrettyName, "Visual Studio Code");
    else if(ffStrbufIgnCaseEqualS(&result->terminalPrettyName, "explorer"))
        ffStrbufSetStatic(&result->terminalPrettyName, "Windows Explorer");
    else if(ffStrbufEqualS(&result->terminalPrettyName, "wezterm-gui"))
        ffStrbufSetStatic(&result->terminalPrettyName, "WezTerm");

    return ppid;
}

bool fftsGetTerminalVersion(FFstrbuf* processName, FFstrbuf* exe, FFstrbuf* version);

const FFTerminalShellResult* ffDetectTerminalShell(void)
{
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

    uint32_t ppid;
    if(!getProcessInfo(0, &ppid, NULL, NULL, NULL))
        goto exit;

    ppid = getShellInfo(&result, ppid);
    if(ppid)
        getTerminalInfo(&result, ppid);

    if(result.terminalProcessName.length == 0)
        getTerminalFromEnv(&result);

    ffStrbufInit(&result.terminalVersion);
    fftsGetTerminalVersion(&result.terminalProcessName, &result.terminalExe, &result.terminalVersion);

exit:
    ffThreadMutexUnlock(&mutex);
    return &result;
}
