extern "C"
{
#include "wm.h"
#include "common/io.h"
#include "common/windows/version.h"
#include "detection/terminalshell/terminalshell.h"
}

#include "common/windows/com.hpp"
#include "common/windows/util.hpp"

#include <windows.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <propkey.h>

extern "C"
const char* ffDetectWMPlugin(FFstrbuf* pluginName)
{
    DWORD pid = ffDetectTerminal()->pid; // Whatever GUI program
    if (pid == 0) return "Unable to find a GUI program";

    FF_AUTO_CLOSE_FD HANDLE snapshot = NULL;
    while(!(snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid)) && GetLastError() == ERROR_BAD_LENGTH) {}

    if(!snapshot)
        return "CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid) failed";

    if (ffInitCom())
        return "ffInitCom() failed";

    MODULEENTRY32W module;
    module.dwSize = sizeof(module);
    for(BOOL success = Module32FirstW(snapshot, &module); success; success = Module32NextW(snapshot, &module))
    {
        if(wcscmp(module.szModule, L"wbhelp64.dll") == 0 || wcscmp(module.szModule, L"wbhelp.dll") == 0)
        {
            IShellItem2* shellItem = NULL;
            if (FAILED(SHCreateItemFromParsingName(module.szExePath, NULL, IID_IShellItem2, (void**) &shellItem)))
                continue;

            on_scope_exit destroyShellItem([=] { shellItem->Release(); });

            wchar_t* desc = NULL;
            if (FAILED(shellItem->GetString(PKEY_FileDescription, &desc)))
                continue;

            on_scope_exit destroyDesc([=] { CoTaskMemFree(desc); });

            if (wcscmp(desc, L"WindowBlinds Helper DLL") == 0)
            {
                ffStrbufSetStatic(pluginName, "WindowBlinds");
                break;
            }
        }
    }
    return NULL;
}

const char* ffDetectWMVersion(const FFstrbuf* wmName, FFstrbuf* result, FF_MAYBE_UNUSED FFWMOptions* options)
{
    if (!wmName)
        return "No WM detected";

    if (ffStrbufEqualS(wmName, "dwm.exe"))
    {
        PWSTR pPath = NULL;
        if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_System, KF_FLAG_DEFAULT, NULL, &pPath)))
        {
            wchar_t fullPath[MAX_PATH];
            wcscpy(fullPath, pPath);
            wcscat(fullPath, L"\\dwm.exe");
            ffGetFileVersion(fullPath, NULL, result);
        }
        CoTaskMemFree(pPath);
        return NULL;
    }
    return "Not supported on this platform";
}
