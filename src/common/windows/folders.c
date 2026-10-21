#include "folders.h"
#include "common/debug.h"
#include "common/mallocHelper.h"
#include "common/windows/nt.h"
#include "common/windows/registry.h"
#include "common/windows/unicode.h"

#include <ntstatus.h>

// Where the shell keeps the values SHGetKnownFolderPath() would compute.
//
// `User Shell Folders` is the store folder redirection writes to -- HKCU for the user's own
// folders, HKLM for the machine-wide ones -- and it holds the *unexpanded* form, which is why the
// environment expansion below is not optional. `ProfileList\<SID>\ProfileImagePath` is what the
// logon session recorded as the profile directory, and is read by ffGetProfilePath().
#define FF_FOLDERS_HKCU_SHELL_FOLDERS L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"
#define FF_FOLDERS_HKLM_SHELL_FOLDERS L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"
#define FF_FOLDERS_HKLM_CURRENT_VERSION L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
#define FF_FOLDERS_HKLM_PROFILE_LIST L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"

// The environment fallbacks. Two of the registry values above are self-referential -- `Common
// AppData` is literally `%ProgramData%` -- so these are the same answer, not a different one.
#define FF_FOLDERS_ENV_LOCAL_APP_DATA L"%LOCALAPPDATA%"
#define FF_FOLDERS_ENV_ROAMING_APP_DATA L"%APPDATA%"
#define FF_FOLDERS_ENV_PROGRAM_DATA L"%ProgramData%"
#define FF_FOLDERS_ENV_PROGRAM_FILES L"%ProgramFiles%"

// Every folder is resolved once and then kept. ffGetKnownFolderPath() is called about a dozen times
// per process -- getCacheDir, getConfigDirs and getDataDirs all ask for the same folders -- and
// each answer is a pure function of the registry and this process' environment. `states` separates
// "not looked up yet" from "looked up and not there", which the path itself cannot: an empty
// FFstrbuf is a valid answer.
//
// No lock guards this: the only thread fastfetch ever creates is the networking module's
// asynchronous connect, and no module that resolves a known folder runs on it.
typedef enum FFFolderState : uint8_t {
    FF_FOLDER_UNRESOLVED,
    FF_FOLDER_FOUND,
    FF_FOLDER_MISSING,
} FFFolderState;

static FFstrbuf folderPaths[FF_KNOWN_FOLDER_COUNT];
static FFFolderState folderStates[FF_KNOWN_FOLDER_COUNT];

// Expands `%NAME%` placeholders from this process' environment block.
//
// The work is done in UTF-16 so that a user name outside the ANSI code page survives: getenv()
// goes through the CRT's narrow copy of the environment and would mangle it.
//
// RtlExpandEnvironmentStrings() substitutes nothing and still reports success for a variable it
// cannot find (measured), so a leftover '%' is treated as a failure. A directory name that
// legitimately contains one is a false negative, and the only thing that costs is the caller's
// fallback.
static bool expandEnv(const wchar_t* source, uint32_t sourceLen, FFstrbuf* result) {
    // Asking for the required length is done with a null destination, which the call reports as
    // STATUS_BUFFER_TOO_SMALL -- a *failure* status that still fills in `size`. The status is
    // therefore deliberately not checked here; a call that writes nothing leaves `size` at zero.
    SIZE_T size = 0; // in characters, including the null terminator
    (void) RtlExpandEnvironmentStrings(nullptr, source, sourceLen, nullptr, 0, &size);
    if (size == 0) {
        return false;
    }

    FF_AUTO_FREE wchar_t* expanded = (wchar_t*) malloc(size * sizeof(wchar_t));
    SIZE_T written = 0;
    if (!NT_SUCCESS(RtlExpandEnvironmentStrings(nullptr, source, sourceLen, expanded, size, &written)) || written == 0) {
        return false;
    }

    if (wcschr(expanded, L'%')) {
        return false;
    }

    ffStrbufSetNWS(result, (uint32_t) (written - 1), expanded);
    return true;
}

static bool expandEnvZ(const wchar_t* source, FFstrbuf* result) {
    return expandEnv(source, (uint32_t) wcslen(source), result);
}

// `raw` is the payload of a REG_SZ / REG_EXPAND_SZ value: UTF-16, usually null terminated.
static bool expandBuffer(const FFArgBuffer* raw, FFstrbuf* result) {
    uint32_t length = raw->length / sizeof(wchar_t);
    if (length == 0) {
        return false;
    }

    const wchar_t* valueW = (const wchar_t*) raw->data;
    if (valueW[length - 1] == L'\0') {
        --length;
    }

    return length > 0 && expandEnv(valueW, length, result);
}

// FF_ARG_TYPE_BUFFER rather than FF_ARG_TYPE_STRBUF, because the value has to stay UTF-16 for
// expandEnv(); the strbuf flavour converts it to UTF-8 on the way out. The buffer flavour does not
// report the registry type either, so REG_SZ and REG_EXPAND_SZ are not told apart -- expanding a
// string without placeholders is a no-op, which is what makes that acceptable.
static bool readExpandedValue(HANDLE hKey, const wchar_t* valueNameW, FFstrbuf* result) {
    FFArgBuffer raw = {};
    if (!ffRegReadData(hKey, valueNameW, &raw, nullptr)) {
        return false;
    }
    FF_AUTO_FREE void* data = raw.data;

    return expandBuffer(&raw, result);
}

static bool readExpandedValueFrom(HKEY hRootKey, const wchar_t* subKeyW, const wchar_t* valueNameW, FFstrbuf* result) {
    FF_AUTO_CLOSE_FD HANDLE hKey = nullptr;
    if (!ffRegOpenKeyForRead(hRootKey, subKeyW, &hKey, nullptr)) {
        return false;
    }

    return readExpandedValue(hKey, valueNameW, result);
}

// The SID comes from the caller rather than from a token query here: FFPlatform_windows.c already
// resolves it for `platform->sid`, and the two are the same SID by definition.
bool ffGetProfilePath(const wchar_t* sidW, FFstrbuf* result) {
    assert(sidW);

    FF_AUTO_CLOSE_FD HANDLE hProfileList = nullptr;
    if (!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, FF_FOLDERS_HKLM_PROFILE_LIST, &hProfileList, nullptr)) {
        FF_DEBUG("Cannot open %ls", FF_FOLDERS_HKLM_PROFILE_LIST);
        return false;
    }

    FF_AUTO_CLOSE_FD HANDLE hProfile = nullptr;
    if (!ffRegOpenSubkeyForRead(hProfileList, sidW, &hProfile, nullptr)) {
        FF_DEBUG("No profile is registered for %ls", sidW);
        return false;
    }

    return readExpandedValue(hProfile, L"ProfileImagePath", result);
}

// `Local AppData` and `AppData` share a key, so they are read together.
//
// The names have to be exact, spaces and all: `Local AppData` is `Local`, one space, `AppData` --
// there is no `Local App Data`, and asking for it fails the whole batch below.
static const struct {
    FFKnownFolder folder;
    const wchar_t* valueNameW;
    const wchar_t* envW;
} shellFolderEntries[] = {
    { FF_KNOWN_FOLDER_LOCAL_APP_DATA, L"Local AppData", FF_FOLDERS_ENV_LOCAL_APP_DATA },
    { FF_KNOWN_FOLDER_ROAMING_APP_DATA, L"AppData", FF_FOLDERS_ENV_ROAMING_APP_DATA },
};

// One open plus one NtQueryMultipleValueKey resolves both -- and whichever of the two is asked for
// first caches the pair, which is what the callers want anyway.
//
// NtQueryMultipleValueKey is all-or-nothing: one absent name fails the batch, which would push
// *both* folders to the environment fallback. That is survivable, because these values are
// themselves just `%LOCALAPPDATA%` and `%APPDATA%` expanded -- but it is also invisible, and an
// invisible fallback is how a wrong value name went unnoticed here. So a failed batch is retried
// one value at a time, and the extra queries are only paid when the batch has already failed.
static void resolveUserShellFolders(void) {
    FFArgBuffer buffers[ARRAY_SIZE(shellFolderEntries)] = {};
    FFRegValueArg args[ARRAY_SIZE(shellFolderEntries)];
    for (uint32_t i = 0; i < ARRAY_SIZE(shellFolderEntries); ++i) {
        args[i] = (FFRegValueArg) { .type = FF_ARG_TYPE_BUFFER, .value = &buffers[i], .name = shellFolderEntries[i].valueNameW };
    }

    FF_AUTO_CLOSE_FD HANDLE hKey = nullptr;
    if (ffRegOpenKeyForRead(HKEY_CURRENT_USER, FF_FOLDERS_HKCU_SHELL_FOLDERS, &hKey, nullptr)) {
        if (ffRegReadValues(hKey, ARRAY_SIZE(shellFolderEntries), args, nullptr)) {
            for (uint32_t i = 0; i < ARRAY_SIZE(shellFolderEntries); ++i) {
                if (expandBuffer(&buffers[i], &folderPaths[shellFolderEntries[i].folder])) {
                    folderStates[shellFolderEntries[i].folder] = FF_FOLDER_FOUND;
                }
            }
        } else {
            FF_DEBUG("Batched read of %ls failed; retrying one value at a time", FF_FOLDERS_HKCU_SHELL_FOLDERS);
            for (uint32_t i = 0; i < ARRAY_SIZE(shellFolderEntries); ++i) {
                if (readExpandedValue(hKey, shellFolderEntries[i].valueNameW, &folderPaths[shellFolderEntries[i].folder])) {
                    folderStates[shellFolderEntries[i].folder] = FF_FOLDER_FOUND;
                }
            }
        }
    }

    for (uint32_t i = 0; i < ARRAY_SIZE(shellFolderEntries); ++i) {
        // Every buffer has to be freed even when the read failed part way through, which is why the
        // free is not tied to the success path above.
        free(buffers[i].data);

        FFKnownFolder folder = shellFolderEntries[i].folder;
        if (folderStates[folder] == FF_FOLDER_UNRESOLVED &&
            expandEnvZ(shellFolderEntries[i].envW, &folderPaths[folder])) {
            folderStates[folder] = FF_FOLDER_FOUND;
        }
        if (folderStates[folder] == FF_FOLDER_UNRESOLVED) {
            folderStates[folder] = FF_FOLDER_MISSING;
        }
    }
}

static bool resolveFolder(FFKnownFolder folder) {
    switch (folder) {
        case FF_KNOWN_FOLDER_LOCAL_APP_DATA:
        case FF_KNOWN_FOLDER_ROAMING_APP_DATA:
            resolveUserShellFolders(); // Resolves both of them
            return folderStates[folder] == FF_FOLDER_FOUND;

        case FF_KNOWN_FOLDER_PROGRAM_DATA:
            return readExpandedValueFrom(HKEY_LOCAL_MACHINE, FF_FOLDERS_HKLM_SHELL_FOLDERS, L"Common AppData", &folderPaths[folder]) ||
                expandEnvZ(FF_FOLDERS_ENV_PROGRAM_DATA, &folderPaths[folder]);

        case FF_KNOWN_FOLDER_PROGRAM_FILES:
            return readExpandedValueFrom(HKEY_LOCAL_MACHINE, FF_FOLDERS_HKLM_CURRENT_VERSION, L"ProgramFilesDir", &folderPaths[folder]) ||
                expandEnvZ(FF_FOLDERS_ENV_PROGRAM_FILES, &folderPaths[folder]);

        default:
            assert(false);
            return false;
    }
}

bool ffGetKnownFolderPath(FFKnownFolder folder, FFstrbuf* result) {
    assert(result);
    assert(folder < FF_KNOWN_FOLDER_COUNT);

    if (folderStates[folder] == FF_FOLDER_UNRESOLVED) {
        folderStates[folder] = resolveFolder(folder) ? FF_FOLDER_FOUND : FF_FOLDER_MISSING;
    }

    if (folderStates[folder] != FF_FOLDER_FOUND) {
        return false;
    }

    ffStrbufSet(result, &folderPaths[folder]);
    return true;
}
