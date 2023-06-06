#include "icons.h"
#include "util/windows/registry.h"

const char* ffDetectIcons(FF_MAYBE_UNUSED const FFinstance* instance, FFstrbuf* result)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\ClassicStartMenu", &hKey, NULL))
        return "ffRegOpenKeyForRead(Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\ClassicStartMenu) failed";

    // Whether these icons are hidden
    uint32_t ThisPC = 0, UsersFiles = 0, RemoteNetwork = 0, RecycleBin = 0, ControlPanel = 0;
    ffRegReadUint(hKey, L"{20D04FE0-3AEA-1069-A2D8-08002B30309D}", &ThisPC, NULL);
    ffRegReadUint(hKey, L"{59031a47-3f72-44a7-89c5-5595fe6b30ee}", &UsersFiles, NULL);
    ffRegReadUint(hKey, L"{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}", &RemoteNetwork, NULL);
    ffRegReadUint(hKey, L"{645FF040-5081-101B-9F08-00AA002F954E}", &RecycleBin, NULL);
    ffRegReadUint(hKey, L"{5399E694-6CE5-4D6C-8FCE-1D8870FDCBA0}", &ControlPanel, NULL);

    if (!ThisPC)
        ffStrbufAppendS(result, "This PC, ");
    if (!UsersFiles)
        ffStrbufAppendS(result, "User's Files, ");
    if (!RemoteNetwork)
        ffStrbufAppendS(result, "Remote Network, ");
    if (!RecycleBin)
        ffStrbufAppendS(result, "Recycle Bin, ");
    if (!ControlPanel)
        ffStrbufAppendS(result, "Control Panel");

    ffStrbufTrimRight(result, ' ');
    ffStrbufTrimRight(result, ',');

    return NULL;
}
