#include "wm.h"
#include "common/mallocHelper.h"
#include "common/io.h"
#include "common/library.h"
#include "common/processing.h"
#include "common/windows/nt.h"
#include "common/windows/unicode.h"
#include "common/windows/version.h"

#include <stdalign.h>
#include <windows.h>
#include <ntstatus.h>
#include <shlobj.h>
#include <softpub.h>

typedef enum {
    FF_PROCESS_TYPE_NONE,
    FF_PROCESS_TYPE_SIGNED = 1 << 0,
    FF_PROCESS_TYPE_WINDOWS_STORE = 1 << 1,
    FF_PROCESS_TYPE_GUI = 1 << 2,
    FF_PROCESS_TYPE_CUI = 1 << 3,
} FFProcessType;

static bool verifySignature(const wchar_t* filePath) {
    FF_LIBRARY_LOAD(wintrust, true, "wintrust" FF_LIBRARY_EXTENSION, -1)
    FF_LIBRARY_LOAD_SYMBOL(wintrust, WinVerifyTrustEx, true)

    WINTRUST_FILE_INFO fileInfo = {
        .cbStruct = sizeof(fileInfo),
        .pcwszFilePath = filePath,
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

    LONG status = ffWinVerifyTrustEx(nullptr, &actionID, &trustData);
    trustData.dwStateAction = WTD_STATEACTION_CLOSE;
    ffWinVerifyTrustEx(nullptr, &actionID, &trustData);

    return status == ERROR_SUCCESS;
}

static bool isProcessTrusted(DWORD processId, FFProcessType processType, UNICODE_STRING* buffer, size_t bufSize) {
    FF_AUTO_CLOSE_FD HANDLE hProcess = nullptr;
    if (!NT_SUCCESS(NtOpenProcess(&hProcess, PROCESS_QUERY_LIMITED_INFORMATION, &(OBJECT_ATTRIBUTES) {
                                                                                    .Length = sizeof(OBJECT_ATTRIBUTES),
                                                                                },
            &(CLIENT_ID) { .UniqueProcess = (HANDLE) (uintptr_t) processId }))) {
        return false;
    }

    ULONG size;
    if (!NT_SUCCESS(NtQueryInformationProcess(hProcess, ProcessImageFileNameWin32, buffer, (ULONG) bufSize, &size)) ||
        buffer->Length == 0) {
        return false;
    }
    assert(buffer->MaximumLength >= buffer->Length + 2); // nullptr terminated

    if (processType & FF_PROCESS_TYPE_WINDOWS_STORE) {
        static wchar_t windowsAppsPath[MAX_PATH];
        static uint32_t windowsAppsPathLen;
        if (windowsAppsPathLen == 0) {
            PWSTR pPath = nullptr;
            if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, nullptr, &pPath))) {
                windowsAppsPathLen = (uint32_t) wcslen(pPath);
                memcpy(windowsAppsPath, pPath, windowsAppsPathLen * sizeof(wchar_t));
                memcpy(windowsAppsPath + windowsAppsPathLen, L"\\WindowsApps\\", sizeof(L"\\WindowsApps\\"));
                windowsAppsPathLen += strlen("\\WindowsApps\\");
            } else {
                windowsAppsPathLen = -1u;
            }
            CoTaskMemFree(pPath);
        }
        if (windowsAppsPathLen != -1u &&
            (buffer->Length <= windowsAppsPathLen * sizeof(wchar_t) ||               // Path is too short to be in WindowsApps
                _wcsnicmp(buffer->Buffer, windowsAppsPath, windowsAppsPathLen) != 0) // Path does not start with WindowsApps
        ) {
            return false;
        }
    }

    if (processType & FF_PROCESS_TYPE_SIGNED) {
        if (!verifySignature(buffer->Buffer)) {
            return false;
        }
    }

    if (processType & (FF_PROCESS_TYPE_GUI | FF_PROCESS_TYPE_CUI)) {
        SECTION_IMAGE_INFORMATION info = {};
        if (!NT_SUCCESS(NtQueryInformationProcess(hProcess, ProcessImageInformation, &info, sizeof(info), &size)) ||
            size != sizeof(info)) {
            return false;
        }

        if ((processType & FF_PROCESS_TYPE_GUI) && info.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_GUI) {
            return false;
        }
        if ((processType & FF_PROCESS_TYPE_CUI) && info.SubSystemType != IMAGE_SUBSYSTEM_WINDOWS_CUI) {
            return false;
        }
    }

    return true;
}

#define ffStrEqualNWS(str, compareTo) (_wcsnicmp(str, L##compareTo, sizeof(compareTo) - 1) == 0)

// Returns true if the process was recognized as a WM plugin and pluginName was set.
static bool handleProcess(FFstrbuf* pluginName, uint32_t pid, const UNICODE_STRING* imageName) {
    alignas(UNICODE_STRING) uint8_t buffer[4096];
    UNICODE_STRING* filePath = (UNICODE_STRING*) buffer;

    if (imageName->Length == strlen("FancyWM-GUI.exe") * sizeof(wchar_t) &&
        ffStrEqualNWS(imageName->Buffer, "FancyWM-GUI.exe") &&
        isProcessTrusted(pid, FF_PROCESS_TYPE_WINDOWS_STORE | FF_PROCESS_TYPE_GUI, filePath, sizeof(buffer))) {
        if (instance.config.general.detectVersion && ffGetFileVersion(filePath->Buffer, nullptr, pluginName)) {
            ffStrbufPrependS(pluginName, "FancyWM ");
        } else {
            ffStrbufSetStatic(pluginName, "FancyWM");
        }
        return true;
    } else if (imageName->Length == strlen("glazewm-watcher.exe") * sizeof(wchar_t) &&
        ffStrEqualNWS(imageName->Buffer, "glazewm-watcher.exe") &&
        isProcessTrusted(pid, FF_PROCESS_TYPE_SIGNED | FF_PROCESS_TYPE_GUI, filePath, sizeof(buffer))) {
        if (instance.config.general.detectVersion && ffGetFileVersion(filePath->Buffer, nullptr, pluginName)) {
            ffStrbufPrependS(pluginName, "GlazeWM ");
        } else {
            ffStrbufSetStatic(pluginName, "GlazeWM");
        }
        return true;
    } else if (imageName->Length == strlen("komorebi.exe") * sizeof(wchar_t) &&
        ffStrEqualNWS(imageName->Buffer, "komorebi.exe") &&
        isProcessTrusted(pid, FF_PROCESS_TYPE_CUI, filePath, sizeof(buffer))) {
        if (instance.config.general.detectVersion) {
            FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateNWS(filePath->Length / sizeof(wchar_t), filePath->Buffer);
            if (ffProcessAppendStdOut(pluginName, (char* const[]) {
                                                      path.chars,
                                                      "--version",
                                                      nullptr,
                                                  }) == nullptr) {
                ffStrbufSubstrBeforeFirstC(pluginName, '\n');
            }
        }
        if (pluginName->length == 0) {
            ffStrbufSetStatic(pluginName, "Komorebi");
        }
        return true;
    }

    return false;
}

const char* ffDetectWMPlugin(FFstrbuf* pluginName) {
    if (ffIsSystemBasicProcessInfoAvailable()) {
        // Unlike SystemProcessInformation, SystemBasicProcessInformation only returns processes
        // (no threads), so a single query with a modest buffer is enough.
        ULONG size = 0;
        if (NtQuerySystemInformation(SystemBasicProcessInformation, nullptr, 0, &size) != STATUS_INFO_LENGTH_MISMATCH) {
            goto fallback;
        }
        // The process table may change between the two calls; retry with a larger buffer.
        size += size / 8 + sizeof(SYSTEM_BASICPROCESS_INFORMATION);
        FF_AUTO_FREE SYSTEM_BASICPROCESS_INFORMATION* pstart = malloc(size);

        if (!NT_SUCCESS(NtQuerySystemInformation(SystemBasicProcessInformation, pstart, size, &size))) {
            goto fallback;
        }

        for (auto ptr = pstart;; ptr = (SYSTEM_BASICPROCESS_INFORMATION*) ((uint8_t*) ptr + ptr->NextEntryOffset)) {
            assert(ptr >= pstart && (uint8_t*) ptr < (uint8_t*) pstart + size);
            if (handleProcess(pluginName, (uint32_t)(uintptr_t) ptr->UniqueProcessId, &ptr->ImageName)) {
                return nullptr;
            }
            // The last process in the list always has a NextEntryOffset of 0, even if the buffer was truncated.
            if (!ptr->NextEntryOffset) {
                break;
            }
        }
        return nullptr;
    }

fallback:
    {
        FF_AUTO_FREE SYSTEM_PROCESS_INFORMATION* pstart = nullptr;

        // Multiple attempts in case processes change while
        // we are in the middle of querying them.
        ULONG size = 0;
        for (int attempts = 0;; ++attempts) {
            if (size) {
                pstart = (SYSTEM_PROCESS_INFORMATION*) realloc(pstart, size);
                assert(pstart);
            }
            NTSTATUS status = NtQuerySystemInformation(SystemProcessInformation, pstart, size, &size);
            if (NT_SUCCESS(status)) {
                break;
            } else if (status == STATUS_INFO_LENGTH_MISMATCH && attempts < 4) {
                size += sizeof(SYSTEM_PROCESS_INFORMATION) * 5;
            } else {
                return "NtQuerySystemInformation(SystemProcessInformation) failed";
            }
        }

        for (auto ptr = pstart;; ptr = (SYSTEM_PROCESS_INFORMATION*) ((uint8_t*) ptr + ptr->NextEntryOffset)) {
            assert(ptr->ImageName.Length == 0 || ptr->ImageName.MaximumLength >= ptr->ImageName.Length + 2); // nullptr terminated
            if (handleProcess(pluginName, (uint32_t)(uintptr_t) ptr->UniqueProcessId, &ptr->ImageName)) {
                return nullptr;
            }
            if (ptr->NextEntryOffset == 0) {
                break;
            }
        }
    }

    return nullptr;
}

const char* ffDetectWMVersion(const FFstrbuf* wmName, FFstrbuf* result, [[maybe_unused]] FFWMOptions* options) {
    if (!wmName) {
        return "No WM detected";
    }

    if (ffStrbufEqualS(wmName, "dwm.exe")) {
        PWSTR pPath = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_System, KF_FLAG_DEFAULT, nullptr, &pPath))) {
            wchar_t fullPath[MAX_PATH];
            wcscpy(fullPath, pPath);
            wcscat(fullPath, L"\\dwm.exe");
            ffGetFileVersion(fullPath, nullptr, result);
        }
        CoTaskMemFree(pPath);
        return nullptr;
    }
    return "Not supported on this platform";
}
