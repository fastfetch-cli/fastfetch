#include "icons.h"
#include "util/windows/registry.h"

const char* ffDetectIcons(FFIconsResult* result)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel", &hKey, NULL) &&
       !ffRegOpenKeyForRead(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\ClassicStartMenu", &hKey, NULL))
        return "ffRegOpenKeyForRead(Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\{NewStartPanel|ClassicStartMenu}) failed";

    // Whether these icons are hidden
    uint32_t ThisPC = 1, UsersFiles = 1, RemoteNetwork = 1, RecycleBin = 0 /* Shown by default */, ControlPanel = 1;
    ffRegReadUint(hKey, L"{20D04FE0-3AEA-1069-A2D8-08002B30309D}", &ThisPC, NULL);
    ffRegReadUint(hKey, L"{59031a47-3f72-44a7-89c5-5595fe6b30ee}", &UsersFiles, NULL);
    ffRegReadUint(hKey, L"{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}", &RemoteNetwork, NULL);
    ffRegReadUint(hKey, L"{645FF040-5081-101B-9F08-00AA002F954E}", &RecycleBin, NULL);
    ffRegReadUint(hKey, L"{5399E694-6CE5-4D6C-8FCE-1D8870FDCBA0}", &ControlPanel, NULL);

    if (!ThisPC)
        ffStrbufAppendS(&result->icons1, "This PC, ");
    if (!UsersFiles)
        ffStrbufAppendS(&result->icons1, "User's Files");
    ffStrbufTrimRight(&result->icons1, ' ');
    ffStrbufTrimRight(&result->icons1, ',');

    if (!RemoteNetwork)
        ffStrbufAppendS(&result->icons2, "Remote Network, ");
    if (!RecycleBin)
        ffStrbufAppendS(&result->icons2, "Recycle Bin, ");
    if (!ControlPanel)
        ffStrbufAppendS(&result->icons2, "Control Panel");
    ffStrbufTrimRight(&result->icons2, ' ');
    ffStrbufTrimRight(&result->icons2, ',');

    return NULL;
}
