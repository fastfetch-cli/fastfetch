#include "terminalshell.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "common/thread.h"
#include "util/mallocHelper.h"
#include "util/windows/registry.h"
#include "util/windows/unicode.h"
#include "util/stringUtils.h"

#include <windows.h>
#include <wchar.h>
#include <tlhelp32.h>

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

bool fftsGetShellVersion(FFstrbuf* exe, const char* exeName, const FFstrbuf* exePath, FFstrbuf* version);

static uint32_t getShellInfo(FFShellResult* result, uint32_t pid)
{
    uint32_t ppid = 0;

    while (pid != 0 && ffProcessGetInfoWindows(pid, &ppid, &result->processName, &result->exe, &result->exeName, &result->exePath, NULL))
    {
        ffStrbufSet(&result->prettyName, &result->processName);
        if(ffStrbufEndsWithIgnCaseS(&result->prettyName, ".exe"))
            ffStrbufSubstrBefore(&result->prettyName, result->prettyName.length - 4);

        //Common programs that are between terminal and own process, but are not the shell
        if(
            ffStrbufIgnCaseEqualS(&result->prettyName, "sudo")          ||
            ffStrbufIgnCaseEqualS(&result->prettyName, "su")            ||
            ffStrbufIgnCaseEqualS(&result->prettyName, "gdb")           ||
            ffStrbufIgnCaseEqualS(&result->prettyName, "lldb")          ||
            ffStrbufIgnCaseEqualS(&result->prettyName, "python")        || // python on windows generates shim executables
            ffStrbufIgnCaseEqualS(&result->prettyName, "fastfetch")     || // scoop warps the real binaries with a "shim" exe
            ffStrbufIgnCaseEqualS(&result->prettyName, "flashfetch")    ||
            ffStrbufIgnCaseEqualS(&result->prettyName, "hyfetch")       || // uses fastfetch as backend
            ffStrbufContainIgnCaseS(&result->prettyName, "debug")       ||
            ffStrbufContainIgnCaseS(&result->prettyName, "time")        ||
            ffStrbufStartsWithIgnCaseS(&result->prettyName, "ConEmu") // https://github.com/fastfetch-cli/fastfetch/issues/488#issuecomment-1619982014
        ) {
            ffStrbufClear(&result->processName);
            ffStrbufClear(&result->prettyName);
            ffStrbufClear(&result->exe);
            result->exeName = NULL;
            pid = ppid;
            continue;
        }

        result->pid = pid;
        result->ppid = ppid;

        if(ffStrbufIgnCaseEqualS(&result->prettyName, "explorer"))
        {
            ffStrbufSetS(&result->prettyName, "Windows Explorer"); // Started without shell
            // In this case, terminal process will be created by fastfetch itself.
            ppid = 0;
        }

        break;
    }
    return ppid;
}

static void setShellInfoDetails(FFShellResult* result)
{
    if(ffStrbufIgnCaseEqualS(&result->prettyName, "pwsh"))
        ffStrbufSetS(&result->prettyName, "PowerShell");
    else if(ffStrbufIgnCaseEqualS(&result->prettyName, "powershell"))
        ffStrbufSetS(&result->prettyName, "Windows PowerShell");
    else if(ffStrbufIgnCaseEqualS(&result->prettyName, "powershell_ise"))
        ffStrbufSetS(&result->prettyName, "Windows PowerShell ISE");
    else if(ffStrbufIgnCaseEqualS(&result->prettyName, "cmd"))
    {
        ffStrbufSetS(&result->prettyName, "CMD");

        if (instance.config.general.detectVersion)
        {
            FF_AUTO_CLOSE_FD HANDLE snapshot = NULL;
            while(!(snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, result->pid)) && GetLastError() == ERROR_BAD_LENGTH) {}

            if(snapshot)
            {
                MODULEENTRY32W module;
                module.dwSize = sizeof(module);
                for(BOOL success = Module32FirstW(snapshot, &module); success; success = Module32NextW(snapshot, &module))
                {
                    if(wcsncmp(module.szModule, L"clink_dll_", strlen("clink_dll_")) == 0)
                    {
                        ffStrbufAppendS(&result->prettyName, " (with Clink ");
                        getProductVersion(module.szExePath, &result->prettyName);
                        ffStrbufAppendC(&result->prettyName, ')');
                        break;
                    }
                }
            }
        }
    }
    else if(ffStrbufIgnCaseEqualS(&result->prettyName, "nu"))
        ffStrbufSetS(&result->prettyName, "nushell");
    else if(ffStrbufIgnCaseEqualS(&result->prettyName, "explorer"))
        ffStrbufSetS(&result->prettyName, "Windows Explorer");
}

static bool getTerminalFromEnv(FFTerminalResult* result)
{
    if(
        result->processName.length > 0 &&
        ffStrbufIgnCaseCompS(&result->processName, "explorer") != 0
    ) return false;

    const char* term = getenv("ConEmuPID");

    if(term)
    {
        //ConEmu
        uint32_t pid = (uint32_t) strtoul(term, NULL, 10);
        result->pid = pid;
        if(ffProcessGetInfoWindows(pid, NULL, &result->processName, &result->exe, &result->exeName, &result->exePath, NULL))
        {
            ffStrbufSet(&result->prettyName, &result->processName);
            if(ffStrbufEndsWithIgnCaseS(&result->prettyName, ".exe"))
                ffStrbufSubstrBefore(&result->prettyName, result->prettyName.length - 4);
            return true;
        }
        else
        {
            term = "ConEmu";
        }
    }

    //SSH
    if(getenv("SSH_TTY") != NULL)
        term = getenv("SSH_TTY");

    //Windows Terminal
    if(!term && (
        getenv("WT_SESSION") != NULL ||
        getenv("WT_PROFILE_ID") != NULL
    )) term = "WindowsTerminal";

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
        ffStrbufSetS(&result->processName, term);
        ffStrbufSetS(&result->prettyName, term);
        ffStrbufSetS(&result->exe, term);
        result->exeName = "";
        return true;
    }

    return false;
}

static bool detectDefaultTerminal(FFTerminalResult* result)
{
    wchar_t regPath[128] = L"SOFTWARE\\Classes\\PackagedCom\\ClassIndex\\";
    wchar_t* uuid = regPath + strlen("SOFTWARE\\Classes\\PackagedCom\\ClassIndex\\");
    DWORD bufSize = 80;
    if (RegGetValueW(HKEY_CURRENT_USER, L"Console\\%%Startup", L"DelegationTerminal", RRF_RT_REG_SZ, NULL, uuid, &bufSize) == ERROR_SUCCESS)
    {
        if(wcscmp(uuid, L"{00000000-0000-0000-0000-000000000000}") == 0 || // Let Windows decide
            wcscmp(uuid, L"{B23D10C0-E52E-411E-9D5B-C09FDF709C7D}") == 0) // Conhost
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
                    ffStrbufSetS(&result->processName, "WindowsTerminal.exe");
                    ffStrbufSetS(&result->prettyName, "WindowsTerminal");
                    ffStrbufSetF(&result->exe, "%s\\WindowsApps\\%s\\WindowsTerminal.exe", getenv("ProgramFiles"), path.chars);
                    if(ffPathExists(result->exe.chars, FF_PATHTYPE_FILE))
                    {
                        result->exeName = result->exe.chars + ffStrbufLastIndexC(&result->exe, '\\') + 1;
                        ffStrbufSet(&result->exePath, &result->exe);
                    }
                    else
                    {
                        ffStrbufDestroy(&result->exe);
                        ffStrbufInitMove(&result->exe, &path);
                        result->exeName = "";
                    }
                    return true;
                }
            }
        }
    }

conhost:
    ffStrbufSetF(&result->exe, "%s\\System32\\conhost.exe", getenv("SystemRoot"));
    if(ffPathExists(result->exe.chars, FF_PATHTYPE_FILE))
    {
        ffStrbufSetS(&result->processName, "conhost.exe");
        ffStrbufSetS(&result->prettyName, "conhost");
        result->exeName = result->exe.chars + ffStrbufLastIndexC(&result->exe, '\\') + 1;
        return true;
    }

    ffStrbufClear(&result->exe);
    return false;
}

static uint32_t getTerminalInfo(FFTerminalResult* result, uint32_t pid)
{
    if (getenv("MSYSTEM"))
    {
        // Don't try to detect terminals in MSYS shell
        // It won't work because MSYS doesn't follow process tree of native Windows programs
        return 0;
    }

    uint32_t ppid = 0;
    bool hasGui;

    while (pid != 0 && ffProcessGetInfoWindows(pid, &ppid, &result->processName, &result->exe, &result->exeName, &result->exePath, &hasGui))
    {
        if(!hasGui || ffStrbufIgnCaseEqualS(&result->processName, "far.exe")) // Far includes GUI objects...
        {
            //We are in nested shell
            ffStrbufClear(&result->processName);
            ffStrbufClear(&result->prettyName);
            ffStrbufClear(&result->exe);
            ffStrbufClear(&result->exePath);
            result->exeName = "";
            pid = ppid;
            continue;
        }

        ffStrbufSet(&result->prettyName, &result->processName);
        if(ffStrbufEndsWithIgnCaseS(&result->prettyName, ".exe"))
            ffStrbufSubstrBefore(&result->prettyName, result->prettyName.length - 4);

        if(ffStrbufIgnCaseEqualS(&result->prettyName, "sihost")           ||
            ffStrbufIgnCaseEqualS(&result->prettyName, "explorer")        ||
            ffStrbufIgnCaseEqualS(&result->prettyName, "wininit")
        ) {
            // A CUI program created by Windows Explorer will spawn a conhost as its child.
            // However the conhost process is just a placeholder;
            // The true terminal can be Windows Terminal or others.
            ffStrbufClear(&result->processName);
            ffStrbufClear(&result->prettyName);
            ffStrbufClear(&result->exe);
            ffStrbufClear(&result->exePath);
            result->exeName = "";
            return 0;
        }
        else
        {
            result->pid = pid;
            result->ppid = ppid;
        }

        break;
    }
    return ppid;
}

static void setTerminalInfoDetails(FFTerminalResult* result)
{
    if(ffStrbufIgnCaseEqualS(&result->prettyName, "WindowsTerminal"))
        ffStrbufSetStatic(&result->prettyName, ffStrbufContainIgnCaseS(&result->exe, ".WindowsTerminalPreview_")
            ? "Windows Terminal Preview"
            : "Windows Terminal"
        );
    else if(ffStrbufIgnCaseEqualS(&result->prettyName, "conhost"))
        ffStrbufSetStatic(&result->prettyName, "Windows Console");
    else if(ffStrbufIgnCaseEqualS(&result->prettyName, "Code"))
        ffStrbufSetStatic(&result->prettyName, "Visual Studio Code");
    else if(ffStrbufIgnCaseEqualS(&result->prettyName, "explorer"))
        ffStrbufSetStatic(&result->prettyName, "Windows Explorer");
    else if(ffStrbufEqualS(&result->prettyName, "wezterm-gui"))
        ffStrbufSetStatic(&result->prettyName, "WezTerm");
    else if(ffStrbufIgnCaseEqualS(&result->prettyName, "sshd") || ffStrbufStartsWithIgnCaseS(&result->prettyName, "sshd-"))
    {
        const char* tty = getenv("SSH_TTY");
        if (tty) ffStrbufSetS(&result->prettyName, tty);
    }
}

bool fftsGetTerminalVersion(FFstrbuf* processName, FFstrbuf* exe, FFstrbuf* version);

const FFShellResult* ffDetectShell(void)
{
    static FFShellResult result;
    static bool init = false;
    if(init)
        return &result;
    init = true;

    ffStrbufInit(&result.processName);
    ffStrbufInitA(&result.exe, MAX_PATH);
    result.exeName = "";
    ffStrbufInit(&result.exePath);
    ffStrbufInit(&result.prettyName);
    ffStrbufInit(&result.version);
    result.pid = 0;
    result.ppid = 0;
    result.tty = -1;

    uint32_t ppid;
    if(!ffProcessGetInfoWindows(0, &ppid, NULL, NULL, NULL, NULL, NULL))
        return &result;

    const char* ignoreParent = getenv("FFTS_IGNORE_PARENT");
    if (ignoreParent && ffStrEquals(ignoreParent, "1"))
        ffProcessGetInfoWindows(ppid, &ppid, NULL, NULL, NULL, NULL, NULL);

    ppid = getShellInfo(&result, ppid);

    if (result.processName.length > 0)
    {
        setShellInfoDetails(&result);
        char tmp[MAX_PATH];
        strcpy(tmp, result.exeName);
        char* ext = strrchr(tmp, '.');
        if (ext) *ext = '\0';
        fftsGetShellVersion(&result.exe, tmp, &result.exePath, &result.version);
    }

    return &result;
}

const FFTerminalResult* ffDetectTerminal(void)
{
    static FFTerminalResult result;
    static bool init = false;
    if(init)
        return &result;
    init = true;

    ffStrbufInit(&result.processName);
    ffStrbufInitA(&result.exe, MAX_PATH);
    result.exeName = "";
    ffStrbufInit(&result.exePath);
    ffStrbufInit(&result.prettyName);
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.tty);
    result.pid = 0;
    result.ppid = 0;

    uint32_t ppid = ffDetectShell()->ppid;
    if(ppid)
        getTerminalInfo(&result, ppid);

    if(result.processName.length == 0)
        getTerminalFromEnv(&result);
    if(result.processName.length == 0)
        detectDefaultTerminal(&result);

    if(result.processName.length > 0)
    {
        setTerminalInfoDetails(&result);
        fftsGetTerminalVersion(&result.processName, &result.exe, &result.version);
    }

    return &result;
}
