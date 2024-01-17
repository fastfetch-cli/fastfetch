extern "C"
{
#include "wm.h"
#include "common/io/io.h"
#include "detection/terminalshell/terminalshell.h"
}

#include "util/windows/com.hpp"

#include <utility>
#include <windows.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <Propkey.h>

template <typename Fn>
struct on_scope_exit {
    on_scope_exit(Fn &&fn): _fn(std::move(fn)) {}
    ~on_scope_exit() { this->_fn(); }

private:
    Fn _fn;
};

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
