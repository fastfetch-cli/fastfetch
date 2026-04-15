#include "common/path.h"
#include "common/io.h"
#include "common/arrayUtils.h"

#if !_WIN32
const char* ffFindExecutableInPath(const char* name, FFstrbuf* result) {
    char* path = getenv("PATH");
    if (!path) {
        return "$PATH not set";
    }

    #ifdef _WIN32
    const bool appendExe = !ffStrEndsWithIgnCase(name, ".exe");
    #endif

    for (char* token = path; *token; path = token + 1) {
        token = strchr(path,
    #ifdef _WIN32
            ';'
    #else
            ':'
    #endif
        );
        if (!token) {
            token = path + strlen(path);
        }

        ffStrbufSetNS(result, (uint32_t) (token - path), path);
        ffStrbufEnsureEndsWithC(result,
    #ifdef _WIN32
            '\\'
    #else
            '/'
    #endif
        );
        ffStrbufAppendS(result, name);
    #ifdef _WIN32
        if (appendExe) {
            ffStrbufAppendS(result, ".exe");
        }
        if (!ffPathExists(result->chars, FF_PATHTYPE_FILE)) {
            continue;
        }
    #else
        if (access(result->chars, X_OK) != 0) {
            continue;
        }
    #endif

        return NULL;
    }
    ffStrbufClear(result);
    return "Executable not found";
}
#else
    #include <windows.h>
    #include <winioctl.h>
    #include <errno.h>
    #include <stdalign.h>

const char* ffFindExecutableInPath(const char* name, FFstrbuf* result) {
    char buffer[MAX_PATH + 1];
    DWORD length = SearchPathA(NULL, name, ".exe", sizeof(buffer), buffer, NULL);
    if (length == 0) {
        ffStrbufClear(result);
        return "Executable not found";
    }
    ffStrbufSetS(result, buffer);
    return NULL;
}

static inline int winerr2Errno(DWORD err) {
    switch (err) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_INVALID_NAME:
            return ENOENT;
        case ERROR_ACCESS_DENIED:
        case ERROR_SHARING_VIOLATION:
        case ERROR_LOCK_VIOLATION:
            return EACCES;
        case ERROR_BUFFER_OVERFLOW:
        case ERROR_INSUFFICIENT_BUFFER:
            return ENAMETOOLONG;
        case ERROR_INVALID_PARAMETER:
        case ERROR_NOT_A_REPARSE_POINT:
            return EINVAL;
        default:
            return EIO;
    }
}

char* frealpath(HANDLE hFile, char* resolved_name) {
    if (__builtin_expect(hFile == INVALID_HANDLE_VALUE || !hFile, false)) {
        errno = EINVAL;
        return NULL;
    }

    wchar_t resolvedNameW[MAX_PATH + 4]; /* +4 for "\\\\?\\" prefix */
    DWORD lenW = GetFinalPathNameByHandleW(hFile, resolvedNameW, (DWORD) ARRAY_SIZE(resolvedNameW), FILE_NAME_NORMALIZED);

    if (lenW == 0) {
        errno = winerr2Errno(GetLastError());
        return NULL;
    }
    if (lenW >= ARRAY_SIZE(resolvedNameW)) {
        errno = E2BIG;
        return NULL;
    }
    lenW++; // Include null terminator

    wchar_t* srcW = resolvedNameW;
    DWORD srcLenW = lenW;

    if (srcLenW >= 8 && wcsncmp(resolvedNameW, L"\\\\?\\UNC\\", 8) == 0) {
        /* Convert "\\?\UNC\server\share" to "\\server\share" */
        srcW += 6;
        srcLenW -= 6;
        *srcW = L'\\';
    } else if (srcLenW >= 4 && wcsncmp(resolvedNameW, L"\\\\?\\", 4) == 0) {
        srcW += 4;
        srcLenW -= 4;
    }

    if (resolved_name) {
        ULONG outBytes = 0;
        if (!NT_SUCCESS(RtlUnicodeToUTF8N(resolved_name, MAX_PATH, &outBytes, srcW, (ULONG) (srcLenW * sizeof(wchar_t))))) {
            errno = E2BIG;
            return NULL;
        }

        if (outBytes > MAX_PATH) {
            errno = E2BIG;
            return NULL;
        }

        return resolved_name;
    } else {
        /* UTF-8 worst-case: up to 4 bytes per UTF-16 code unit */
        char tmp[(MAX_PATH + 4) * 4];
        ULONG outBytes = 0;

        if (!NT_SUCCESS(RtlUnicodeToUTF8N(tmp, (ULONG) sizeof(tmp), &outBytes, srcW, (ULONG) (srcLenW * sizeof(wchar_t))))) {
            errno = E2BIG;
            return NULL;
        }

        resolved_name = (char*) malloc(outBytes);
        if (!resolved_name) {
            errno = ENOMEM;
            return NULL;
        }

        memcpy(resolved_name, tmp, outBytes);
        return resolved_name;
    }

    return resolved_name;
}

char* realpath(const char* __restrict file_name, char* __restrict resolved_name) {
    if (!file_name) {
        errno = EINVAL;
        return NULL;
    }

    wchar_t fileNameW[MAX_PATH];
    ULONG lenBytes = 0;

    if (!NT_SUCCESS(RtlUTF8ToUnicodeN(fileNameW, (ULONG) sizeof(fileNameW), &lenBytes, file_name, (ULONG) strlen(file_name) + 1))) {
        errno = EINVAL;
        return NULL;
    }

    FF_AUTO_CLOSE_FD HANDLE hFile = CreateFileW(
        fileNameW,
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        errno = winerr2Errno(GetLastError());
        return NULL;
    }

    return frealpath(hFile, resolved_name);
}

ssize_t freadlink(HANDLE hFile, char* buf, size_t bufsiz) {
    if (__builtin_expect(hFile == INVALID_HANDLE_VALUE || !buf || bufsiz == 0, false)) {
        errno = EINVAL;
        return -1;
    }

    alignas(REPARSE_DATA_BUFFER) BYTE reparseBuf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
    DWORD bytesReturned = 0;
    if (!DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, NULL, 0, reparseBuf, (DWORD) sizeof(reparseBuf), &bytesReturned, NULL)) {
        errno = winerr2Errno(GetLastError());
        return -1;
    }

    REPARSE_DATA_BUFFER* rp = (REPARSE_DATA_BUFFER*) reparseBuf;
    const wchar_t* targetW = NULL;
    USHORT targetBytes = 0;

    if (rp->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
        if (rp->SymbolicLinkReparseBuffer.PrintNameLength > 0) {
            targetW = rp->SymbolicLinkReparseBuffer.PathBuffer +
                (rp->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(wchar_t));
            targetBytes = rp->SymbolicLinkReparseBuffer.PrintNameLength;
        } else {
            targetW = rp->SymbolicLinkReparseBuffer.PathBuffer +
                (rp->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t));
            targetBytes = rp->SymbolicLinkReparseBuffer.SubstituteNameLength;

            if (targetBytes >= 8 &&
                wcsncmp(targetW, L"\\??\\", 4) == 0) {
                targetW += 4;
                targetBytes -= 8;
            }
        }
    } else if (rp->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
        if (rp->MountPointReparseBuffer.PrintNameLength > 0) {
            targetW = rp->MountPointReparseBuffer.PathBuffer +
                (rp->MountPointReparseBuffer.PrintNameOffset / sizeof(wchar_t));
            targetBytes = rp->MountPointReparseBuffer.PrintNameLength;
        } else {
            targetW = rp->MountPointReparseBuffer.PathBuffer +
                (rp->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t));
            targetBytes = rp->MountPointReparseBuffer.SubstituteNameLength;

            if (targetBytes >= 8 &&
                wcsncmp(targetW, L"\\??\\", 4) == 0) {
                targetW += 4;
                targetBytes -= 8;
            }
        }
    } else {
        errno = EINVAL;
        return -1;
    }

    ULONG outBytes = 0;
    if (!NT_SUCCESS(RtlUnicodeToUTF8N(buf, (ULONG) bufsiz, &outBytes, targetW, targetBytes))) {
        errno = E2BIG;
        return -1;
    }

    // Not null-terminated
    return (ssize_t) outBytes;
}

ssize_t readlink(const char* path, char* buf, size_t bufsiz) {
    if (!path || !buf || bufsiz == 0) {
        errno = EINVAL;
        return -1;
    }

    wchar_t pathW[MAX_PATH];
    ULONG pathWBytes = 0;
    if (!NT_SUCCESS(RtlUTF8ToUnicodeN(pathW, (ULONG) sizeof(pathW), &pathWBytes, path, (ULONG) strlen(path) + 1))) {
        errno = EINVAL;
        return -1;
    }

    FF_AUTO_CLOSE_FD HANDLE hFile = CreateFileW(
        pathW,
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        errno = winerr2Errno(GetLastError());
        return -1;
    }

    return freadlink(hFile, buf, bufsiz);
}
#endif
