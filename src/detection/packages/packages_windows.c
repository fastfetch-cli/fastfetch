#include "packages.h"
#include "common/processing.h"
#include "common/strutil.h"
#include "common/path.h"
#include "common/windows/unicode.h"
#include "common/windows/registry.h"
#include "common/mallocHelper.h"
#include "common/io.h"

#include <stdalign.h>
#include <windows.h>
#include "common/windows/nt.h"
#include <ntstatus.h>
#include <shlobj.h>

static uint32_t getNumElements(const char* searchPath, DWORD type, const wchar_t* ignore) {
    FF_AUTO_CLOSE_FD HANDLE dfd = CreateFileA(searchPath, FILE_LIST_DIRECTORY | SYNCHRONIZE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (dfd == INVALID_HANDLE_VALUE) {
        return 0;
    }

    bool flag = ignore == nullptr;
    uint32_t counter = 0;
    alignas(8) uint8_t buffer[64 * 1024];
    BOOLEAN firstScan = TRUE;

    size_t ignoreLen = ignore ? wcslen(ignore) : 0;

    while (true) {
        IO_STATUS_BLOCK ioStatus = {};
        NTSTATUS status = NtQueryDirectoryFile(
            dfd,
            nullptr,
            nullptr,
            nullptr,
            &ioStatus,
            buffer,
            ARRAY_SIZE(buffer),
            FileDirectoryInformation,
            FALSE,
            nullptr,
            firstScan);
        firstScan = FALSE;

        if (!NT_SUCCESS(status) && status != STATUS_BUFFER_OVERFLOW) {
            break;
        }

        for (FILE_DIRECTORY_INFORMATION* entry = (FILE_DIRECTORY_INFORMATION*) buffer;
            ;
            entry = (FILE_DIRECTORY_INFORMATION*) ((uint8_t*) entry + entry->NextEntryOffset)) {
            if (!(entry->FileAttributes & type)) {
                continue;
            }

            if (!flag &&
                ignoreLen == entry->FileNameLength / sizeof(*entry->FileName) &&
                _wcsnicmp(entry->FileName, ignore, ignoreLen) == 0) {
                flag = true;
                continue;
            }

            counter++;

            if (entry->NextEntryOffset == 0) {
                break;
            }
        }

        if (status == STATUS_SUCCESS) {
            break; // No next page
        }
    }

    if (type == FILE_ATTRIBUTE_DIRECTORY && counter >= 2) {
        counter -= 2; // accounting for . and ..
    }

    return counter;
}

static inline void wrapYyjsonFree(yyjson_doc** doc) {
    assert(doc);
    if (*doc) {
        yyjson_doc_free(*doc);
    }
}

static void detectScoop(FFPackagesResult* result) {
    FF_STRBUF_AUTO_DESTROY scoopPath = ffStrbufCreateA(MAX_PATH + 3);
    ffStrbufAppend(&scoopPath, &instance.state.platform.homeDir);
    ffStrbufAppendS(&scoopPath, ".config/scoop/config.json");

    yyjson_val* root = nullptr;

    [[gnu::cleanup(wrapYyjsonFree)]] yyjson_doc* doc = yyjson_read_file(scoopPath.chars, 0, nullptr, nullptr);
    if (doc) {
        root = yyjson_doc_get_root(doc);
        if (!yyjson_is_obj(root)) {
            root = nullptr;
        }
    }

    {
        ffStrbufClear(&scoopPath);
        if (root) {
            ffStrbufSetJsonVal(&scoopPath, yyjson_obj_get(root, "root_path"));
        }
        if (scoopPath.length == 0) {
            ffStrbufSet(&scoopPath, &instance.state.platform.homeDir);
            ffStrbufAppendS(&scoopPath, "/scoop");
        }
        ffStrbufAppendS(&scoopPath, "/apps/");
        result->scoopUser = getNumElements(scoopPath.chars, FILE_ATTRIBUTE_DIRECTORY, L"scoop");
    }

    {
        ffStrbufClear(&scoopPath);
        if (root) {
            ffStrbufSetJsonVal(&scoopPath, yyjson_obj_get(root, "global_path"));
        }
        if (scoopPath.length == 0) {
            PWSTR pPath = nullptr;
            if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_ProgramData, KF_FLAG_DEFAULT, nullptr, &pPath))) {
                ffStrbufSetWS(&scoopPath, pPath);
                CoTaskMemFree(pPath);
            }
            ffStrbufAppendS(&scoopPath, "/scoop");
        }
        ffStrbufAppendS(&scoopPath, "/apps/");
        result->scoopGlobal = getNumElements(scoopPath.chars, FILE_ATTRIBUTE_DIRECTORY, L"scoop");
    }
}

static void detectChoco([[maybe_unused]] FFPackagesResult* result) {
    const char* chocoInstall = getenv("ChocolateyInstall");
    if (!chocoInstall || chocoInstall[0] == '\0') {
        return;
    }

    char chocoPath[MAX_PATH + 3];
    char* pend = ffStrCopy(chocoPath, chocoInstall, ARRAY_SIZE(chocoPath));
    ffStrCopy(pend, "/lib/", ARRAY_SIZE(chocoPath) - (size_t) (pend - chocoPath));
    result->choco = getNumElements(chocoPath, FILE_ATTRIBUTE_DIRECTORY, L"choco");
}

static void detectPacman(FFPackagesResult* result) {
    const char* msystemPrefix = getenv("MSYSTEM_PREFIX");
    if (!msystemPrefix) {
        return;
    }

    // MSYS2
    char pacmanPath[MAX_PATH + 3];
    char* pend = ffStrCopy(pacmanPath, msystemPrefix, ARRAY_SIZE(pacmanPath));
    ffStrCopy(pend, "/../var/lib/pacman/local/", ARRAY_SIZE(pacmanPath) - (size_t) (pend - pacmanPath));
    result->pacman = getNumElements(pacmanPath, FILE_ATTRIBUTE_DIRECTORY, nullptr);
}

// The newest last-write time among the registry keys that hold the set of installed programs.
//
// winget is the only package manager on Windows whose count cannot be read from a local index:
// `winget list` has to spawn a process (~1.5 s), so its result is cached. The obvious cache key --
// the modification time of winget's own database -- does not work. `installed.db` is a
// `PackageTrackingCatalog`: it only records the installs winget performed itself. Measured on a
// machine with 101 listed packages: it holds 46 ids, and 33 of the listed ones are absent from it.
// Uninstalling a package through Settings > Apps leaves the file completely untouched (same mtime,
// same size, same row count) while `winget list` loses a row.
//
// The sources `winget list` correlates against are the ARP registry and the MSIX package registry.
// Adding or removing a program bumps the last-write time of the parent key it lives under, so the
// newest of these four changes exactly when the count may have changed.
// All four must be read: a program may register in either ARP view, or be an MSIX package.
//
// The keys are read through registry.h, like every other Windows detection module does. There the
// 32-bit view has to be addressed as a path: NtOpenKey ignores KEY_WOW64_32KEY -- it hands back the
// 64-bit key with the flag set just as with it cleared (measured), the redirect is done by the Win32
// Reg* wrappers. What the redirector resolves to is the physical `SOFTWARE\WOW6432Node\...`, so
// opening that path is equivalent to asking for KEY_WOW64_32KEY.
static uint64_t getInstalledProgramsFingerprint(void) {
    static const struct {
        HKEY root;
        const wchar_t* path;
    } locations[] = {
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall" },
#ifdef _WIN64
        // A 64-bit program may still register in the 32-bit view -- the installer decides. Geekbench 6
        // does, and reading only the 64-bit view misses its uninstall entirely.
        //
        // A 32-bit fastfetch is supported on 32-bit Windows only (a 32-bit process on 64-bit Windows
        // would need KEY_WOW64_64KEY, which this API cannot express): there the key above is the one
        // and only ARP view, and this one does not exist.
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall" },
#endif
        { HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall" },
        { HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModel\\StateRepository\\Cache\\Package\\Data" },
    };

    uint64_t newest = 0;
    for (size_t i = 0; i < ARRAY_SIZE(locations); ++i) {
        FF_AUTO_CLOSE_FD HANDLE hKey = nullptr;
        if (!ffRegOpenKeyForRead(locations[i].root, locations[i].path, &hKey, nullptr)) {
            continue;
        }

        KEY_CACHED_INFORMATION info = {};
        if (ffRegQueryKey(hKey, &info, nullptr) && (uint64_t) info.LastWriteTime.QuadPart > newest) {
            newest = (uint64_t) info.LastWriteTime.QuadPart;
        }
    }

    return newest;
}

static void detectWinget(FFPackagesResult* result) {
    // Why not read winget's own database instead of shelling out?
    // `%LOCALAPPDATA%\Packages\Microsoft.DesktopAppInstaller_8wekyb3d8bbwe\LocalState\<source>\installed.db`
    // is a `PackageTrackingCatalog`: it records the install / uninstall actions winget performed for that
    // source, not a snapshot of what is currently installed. Uninstalling with anything but winget leaves
    // the record behind forever, so counting it also counts packages that are long gone. Its schema is
    // private and versioned (1.3, while the source index is 2.0), winget opens it ReadWrite while running,
    // and reading it would drag in a SQLite dependency.
    // The authoritative set of installed packages comes from the ARP registry and MSIX, which is exactly
    // what `winget list` enumerates before correlating it against the read-only source index.

    // The fingerprint is read once, before the process runs, and reused as the cache key on a miss.
    // `winget list` writes to none of those keys, so a stored key stays valid until a program changes.
    const uint64_t cacheKey = getInstalledProgramsFingerprint();

    FF_STRBUF_AUTO_DESTROY cacheDir = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY cacheContent = ffStrbufCreate();

    uint32_t count;
    if (ffPackagesReadCacheKey(&cacheDir, &cacheContent, cacheKey, "winget", &count)) {
        result->winget = count;
        return;
    }

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buffer, (char*[]) {
                                           "winget.exe",
                                           "list",
                                           // Without `--source winget`, winget also lists every package installed by
                                           // other means (ARP / MSIX), which are not winget packages at all.
                                           // It also skips the msstore HTTP round-trips, which are the main reason
                                           // why `winget list` is slow and its latency unpredictable.
                                           "--source",
                                           "winget",
                                           "--disable-interactivity",
                                           // Accepting is a no-op once it has been done. Without it, `winget list`
                                           // exits with code 1 on a machine that has never accepted the source
                                           // agreements, and this module would report no packages at all.
                                           "--accept-source-agreements",
                                           nullptr,
                                       })) {
        // A failed detection is not cached: only ffPackagesWriteCache() writes, and it is not reached.
        return;
    }

    uint32_t index = ffStrbufFirstIndexS(&buffer, "--\r\n"); // Ignore garbage and table headers
    if (index == buffer.length) {
        return;
    }

    count = 0;
    for (
        index += strlen("--\r\n");
        (index = ffStrbufNextIndexC(&buffer, index, '\n')) < buffer.length;
        ++index) {
        ++count;
    }

    if (buffer.chars[buffer.length - 1] != '\n') { // count last line
        ++count;
    }

    result->winget = count;

    ffPackagesWriteCache(&cacheDir, &cacheContent, count);
}

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options) {
    if (FF_PACKAGES_IS_ENABLED(options, SCOOP)) {
        detectScoop(result);
    }
    if (FF_PACKAGES_IS_ENABLED(options, CHOCO)) {
        detectChoco(result);
    }
    if (FF_PACKAGES_IS_ENABLED(options, PACMAN)) {
        detectPacman(result);
    }
    if (FF_PACKAGES_IS_ENABLED(options, WINGET)) {
        detectWinget(result);
    }
}
