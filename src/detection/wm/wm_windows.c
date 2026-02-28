#include "wm.h"
#include "common/mallocHelper.h"
#include "common/io.h"
#include "common/processing.h"
#include "common/windows/nt.h"
#include "common/windows/unicode.h"
#include "common/windows/version.h"

#include <stdalign.h>
#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>
#include <shlobj.h>
#include <softpub.h>

typedef enum {
    FF_PROCESS_TYPE_NONE,
    FF_PROCESS_TYPE_SIGNED = 1 << 0,
    FF_PROCESS_TYPE_WINDOWS_STORE = 1 << 1,
    FF_PROCESS_TYPE_GUI = 1 << 2,
    FF_PROCESS_TYPE_CUI = 1 << 3,
} FFProcessType;

bool isProcessTrusted(DWORD processId, FFProcessType processType, UNICODE_STRING* buffer, size_t bufSize)
{
    FF_AUTO_CLOSE_FD HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (!hProcess)
        return false;

    ULONG size;
    if(!NT_SUCCESS(NtQueryInformationProcess(hProcess, ProcessImageFileNameWin32, buffer, (ULONG) bufSize, &size)) ||
        buffer->Length == 0) return false;
    assert(buffer->MaximumLength >= buffer->Length + 2); // NULL terminated

    if (processType & FF_PROCESS_TYPE_WINDOWS_STORE)
    {
        static wchar_t windowsAppsPath[MAX_PATH];
        static uint32_t windowsAppsPathLen;
        if (windowsAppsPathLen == 0)
        {
            PWSTR pPath = NULL;
            if(SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &pPath)))
            {
                windowsAppsPathLen = (uint32_t) wcslen(pPath);
                memcpy(windowsAppsPath, pPath, windowsAppsPathLen * sizeof(wchar_t));
                memcpy(windowsAppsPath + windowsAppsPathLen, L"\\WindowsApps\\", sizeof(L"\\WindowsApps\\"));
                windowsAppsPathLen += strlen("\\WindowsApps\\");
            }
            else
            {
                windowsAppsPathLen = -1u;
            }
            CoTaskMemFree(pPath);
        }
        if (windowsAppsPathLen != -1u &&
            buffer->Length > windowsAppsPathLen * sizeof(wchar_t) &&
            _wcsnicmp(buffer->Buffer, windowsAppsPath, windowsAppsPathLen) != 0
        ) return false;
    }

    if (processType & FF_PROCESS_TYPE_SIGNED)
    {
        WINTRUST_FILE_INFO fileInfo = {
            .cbStruct = sizeof(fileInfo),
            .pcwszFilePath = buffer->Buffer,
        };

        GUID actionID = WINTRUST_ACTION_GENERIC_VERIFY_V2;

        WINTRUST_DATA trustData = {
            .cbStruct = sizeof(trustData),
            .dwUIChoice = WTD_UI_NONE,
            .fdwRevocationChecks = WTD_REVOKE_NONE,
            .dwUnionChoice = WTD_CHOICE_FILE,
            .pFile = &fileInfo,
            .dwStateAction = WTD_STATEACTION_VERIFY,
            .dwProvFlags = WTD_SAFER_FLAG,
        };

        LONG status = WinVerifyTrustEx(NULL, &actionID, &trustData);
        trustData.dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrust(NULL, &actionID, &trustData);

        if (status != ERROR_SUCCESS)
            return false;
    }

    if (processType & (FF_PROCESS_TYPE_GUI | FF_PROCESS_TYPE_CUI))
    {
        SECTION_IMAGE_INFORMATION info = {};
        if(!NT_SUCCESS(NtQueryInformationProcess(hProcess, ProcessImageInformation, &info, sizeof(info), &size)) ||
            size != sizeof(info)) return false;

        if ((processType & FF_PROCESS_TYPE_GUI) && info.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_GUI)
            return false;
        if ((processType & FF_PROCESS_TYPE_CUI) && info.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_CUI)
            return false;
    }

    return true;
}

const char* ffDetectWMPlugin(FFstrbuf* pluginName)
{
    alignas(UNICODE_STRING) uint8_t buffer[4096];
    UNICODE_STRING* filePath = (UNICODE_STRING*) buffer;
    SYSTEM_PROCESS_INFORMATION* FF_AUTO_FREE pstart = NULL;

    // Multiple attempts in case processes change while
    // we are in the middle of querying them.
    ULONG size = 0;
    for (int attempts = 0;; ++attempts)
    {
        if (size)
        {
            pstart = (SYSTEM_PROCESS_INFORMATION*)realloc(pstart, size);
            assert(pstart);
        }
        NTSTATUS status = NtQuerySystemInformation(SystemProcessInformation, pstart, size, &size);
        if(NT_SUCCESS(status))
            break;
        else if(status == STATUS_INFO_LENGTH_MISMATCH && attempts < 4)
            size += sizeof(SYSTEM_PROCESS_INFORMATION) * 5;
        else
            return "NtQuerySystemInformation(SystemProcessInformation) failed";
    }

    for (SYSTEM_PROCESS_INFORMATION* ptr = pstart; ptr->NextEntryOffset; ptr = (SYSTEM_PROCESS_INFORMATION*)((uint8_t*)ptr + ptr->NextEntryOffset))
    {
        assert(ptr->ImageName.Length == 0 || ptr->ImageName.MaximumLength >= ptr->ImageName.Length + 2); // NULL terminated
        if (ptr->ImageName.Length == strlen("FancyWM-GUI.exe") * sizeof(wchar_t) &&
            memcmp(ptr->ImageName.Buffer, L"FancyWM-GUI.exe", ptr->ImageName.Length) == 0 &&
            isProcessTrusted((DWORD) (uintptr_t) ptr->UniqueProcessId, FF_PROCESS_TYPE_WINDOWS_STORE | FF_PROCESS_TYPE_GUI, filePath, sizeof(buffer))
        ) {
            if (ffGetFileVersion(filePath->Buffer, NULL, pluginName))
                ffStrbufPrependS(pluginName, "FancyWM ");
            else
                ffStrbufSetStatic(pluginName, "FancyWM");
            break;
        }
        else if (ptr->ImageName.Length == strlen("glazewm-watcher.exe") * sizeof(wchar_t) &&
            memcmp(ptr->ImageName.Buffer, L"glazewm-watcher.exe", ptr->ImageName.Length) == 0 &&
            isProcessTrusted((DWORD) (uintptr_t) ptr->UniqueProcessId, FF_PROCESS_TYPE_SIGNED | FF_PROCESS_TYPE_GUI, filePath, sizeof(buffer))
        ) {
            if (ffGetFileVersion(filePath->Buffer, NULL, pluginName))
                ffStrbufPrependS(pluginName, "GlazeWM ");
            else
                ffStrbufSetStatic(pluginName, "GlazeWM");
            break;
        }
        else if (ptr->ImageName.Length == strlen("komorebi.exe") * sizeof(wchar_t) &&
            memcmp(ptr->ImageName.Buffer, L"komorebi.exe", ptr->ImageName.Length) == 0 &&
            isProcessTrusted((DWORD) (uintptr_t) ptr->UniqueProcessId, FF_PROCESS_TYPE_CUI, filePath, sizeof(buffer))
        ) {
            FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateNWS(filePath->Length / sizeof(wchar_t), filePath->Buffer);
            if (ffProcessAppendStdOut(pluginName, (char *const[]) {
                path.chars,
                "--version",
                NULL,
            }) == NULL)
                ffStrbufSubstrBeforeFirstC(pluginName, '\n');
            else
                ffStrbufSetStatic(pluginName, "Komorebi");
            break;
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
        if(SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_System, KF_FLAG_DEFAULT, NULL, &pPath)))
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
