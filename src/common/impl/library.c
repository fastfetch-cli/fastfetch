#include "fastfetch.h"
#include "common/library.h"

#if _WIN32
    #include "common/debug.h"
    #include "common/windows/nt.h"
    #include <errno.h>
    #include <ntstatus.h>
#endif

#ifndef FF_DISABLE_DLOPEN

    #include <stdarg.h>

    // Clang doesn't define __SANITIZE_ADDRESS__ but defines __has_feature(address_sanitizer)
    #if !defined(__SANITIZE_ADDRESS__) && defined(__has_feature)
        #if __has_feature(address_sanitizer)
            #define __SANITIZE_ADDRESS__
        #endif
    #endif

    #ifndef FF_DLOPEN_FLAGS
        #ifdef __SANITIZE_ADDRESS__
            #define FF_DLOPEN_FLAGS RTLD_LAZY | RTLD_NODELETE
        #else
            #define FF_DLOPEN_FLAGS RTLD_LAZY
        #endif
    #endif

static void* libraryLoad(const char* path, int maxVersion) {
    void* result = dlopen(path, FF_DLOPEN_FLAGS);

    #if _WIN32

    // libX.dll.1 never exists on Windows, while libX-1.dll may exist
    FF_UNUSED(maxVersion)

    if (result != NULL) {
        return result;
    }

    uint32_t pathLen = ffStrbufLastIndexC(&instance.state.platform.exePath, '/');
    if (pathLen == instance.state.platform.exePath.length) {
        return result;
    }

    char absPath[MAX_PATH * 2];
    strcpy(mempcpy(absPath, instance.state.platform.exePath.chars, pathLen + 1), path);
    return dlopen(absPath, FF_DLOPEN_FLAGS);

    #else

    if (result != NULL || maxVersion < 0) {
        return result;
    }

    FF_STRBUF_AUTO_DESTROY pathbuf = ffStrbufCreateA(64);
    ffStrbufAppendS(&pathbuf, path);
    ffStrbufAppendC(&pathbuf, '.');

    for (int i = maxVersion; i >= 0; --i) {
        uint32_t originalLength = pathbuf.length;
        ffStrbufAppendSInt(&pathbuf, i);

        result = dlopen(pathbuf.chars, FF_DLOPEN_FLAGS);
        if (result != NULL) {
            break;
        }

        ffStrbufSubstrBefore(&pathbuf, originalLength);
    }

    #endif

    return result;
}

void* ffLibraryLoad(const char* path, int maxVersion, ...) {
    void* result = libraryLoad(path, maxVersion);

    if (!result) {
        va_list defaultNames;
        va_start(defaultNames, maxVersion);

        do {
            const char* pathRest = va_arg(defaultNames, const char*);
            if (pathRest == NULL) {
                break;
            }

            int maxVersionRest = va_arg(defaultNames, int);
            result = libraryLoad(pathRest, maxVersionRest);
        } while (!result);

        va_end(defaultNames);
    }

    return result;
}

#endif

#if _WIN32

void* dlopen(const char* path, FF_A_UNUSED int mode) {
    wchar_t pathW[MAX_PATH + 1];
    ULONG pathWBytes = 0;

    NTSTATUS status = RtlUTF8ToUnicodeN(pathW, sizeof(pathW), &pathWBytes, path, (uint32_t) strlen(path) + 1);
    if (!NT_SUCCESS(status)) {
        FF_DEBUG("RtlUTF8ToUnicodeN failed for path %s with status 0x%08lX: %s", path, status, ffDebugNtStatus(status));
        return NULL;
    }

    PVOID module = NULL;
    status = LdrLoadDll(NULL, NULL, &(UNICODE_STRING) {
                                        .Length = (USHORT) (pathWBytes - sizeof(wchar_t)), // Exclude null terminator
                                        .MaximumLength = (USHORT) pathWBytes,
                                        .Buffer = pathW,
                                    },
        &module);

    if (!NT_SUCCESS(status)) {
        FF_DEBUG("LdrLoadDll failed for path %s with status 0x%08lX: %s", path, status, ffDebugNtStatus(status));
        return NULL;
    }

    return module;
}

int dlclose(void* handle) {
    NTSTATUS status = LdrUnloadDll(handle);
    if (!NT_SUCCESS(status)) {
        FF_DEBUG("LdrUnloadDll failed for handle %p with status 0x%08lX: %s", handle, status, ffDebugNtStatus(status));
        return -1;
    }
    return 0;
}

void* dlsym(void* handle, const char* symbol) {
    void* address;
    USHORT symbolBytes = (USHORT) (strlen(symbol) + 1);
    NTSTATUS status = LdrGetProcedureAddress(handle, &(ANSI_STRING) {
                                                         .Length = symbolBytes - sizeof(char),
                                                         .MaximumLength = symbolBytes,
                                                         .Buffer = (char*) symbol,
                                                     },
        0,
        &address);
    if (!NT_SUCCESS(status)) {
        FF_DEBUG("LdrGetProcedureAddress failed for symbol %s with status 0x%08lX: %s", symbol, status, ffDebugNtStatus(status));
        return NULL;
    }
    return address;
}

void* ffLibraryGetModule(const wchar_t* libraryFileName) {
    assert(libraryFileName != NULL && "Use \"ffGetPeb()->ImageBaseAddress\" instead");

    void* module = NULL;
    USHORT libraryFileNameBytes = (USHORT) (wcslen(libraryFileName) * sizeof(wchar_t) + sizeof(wchar_t));
    NTSTATUS status = LdrGetDllHandle(NULL, NULL, &(UNICODE_STRING) {
                                                      .Length = libraryFileNameBytes - sizeof(wchar_t),
                                                      .MaximumLength = libraryFileNameBytes,
                                                      .Buffer = (wchar_t*) libraryFileName,
                                                  },
        &module);
    if (!NT_SUCCESS(status)) {
        FF_DEBUG("LdrGetDllHandle failed for library %ls with status 0x%08lX: %s", libraryFileName, status, ffDebugNtStatus(status));
        return NULL;
    }
    return module;
}
#endif
