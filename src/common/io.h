#pragma once

#include "common/FFstrbuf.h"
#include "common/FFlist.h"

#ifdef _WIN32
    #include <fileapi.h>
    #include <handleapi.h>
    #include <io.h>
    #include "common/windows/nt.h"
typedef HANDLE FFNativeFD;
    #define FF_INVALID_FD INVALID_HANDLE_VALUE
#else
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #include <errno.h>
    #include <limits.h>
    #include <fcntl.h>
typedef int FFNativeFD;
    #define FF_INVALID_FD (-1)
    // procfs's file can be changed between read calls such as /proc/meminfo and /proc/uptime.
    // one safe way to read correct data is reading the whole file in a single read syscall
    #define PROC_FILE_BUFFSIZ (32 * 1024)
#endif

#ifdef _WIN32
    #ifndef O_CLOEXEC
        #define O_CLOEXEC 0
    #endif
    #ifndef O_RDONLY
        #define O_RDONLY 0
    #endif
    #ifndef O_DIRECTORY
        #define O_DIRECTORY 0200000
    #endif

// Only O_RDONLY is supported
HANDLE openat(HANDLE dfd, const char* fileName, int oflag);
HANDLE openatW(HANDLE dfd, const wchar_t* fileName, uint16_t fileNameLen, bool directory);
#endif

static inline bool ffIsValidNativeFD(FFNativeFD fd) {
#ifndef _WIN32
    return fd >= 0;
#else
    // https://devblogs.microsoft.com/oldnewthing/20040302-00/?p=40443
    return fd != INVALID_HANDLE_VALUE && fd != nullptr;
#endif
}

[[gnu::always_inline, gnu::nonnull(1)]]
static inline void wrapClose(FFNativeFD* pfd) {
    assert(pfd);

    if (ffIsValidNativeFD(*pfd)) {
#ifndef _WIN32
        close(*pfd);
#else
        NtClose(*pfd);
#endif
    }
}
#define FF_AUTO_CLOSE_FD [[gnu::cleanup(wrapClose)]]

static inline FFNativeFD FFUnixFD2NativeFD(int unixfd) {
#ifndef _WIN32
    return unixfd;
#else
    return (FFNativeFD) _get_osfhandle(unixfd);
#endif
}

[[gnu::nonnull(3)]]
static inline bool ffWriteFDData(FFNativeFD fd, size_t dataSize, const void* data) {
#ifndef _WIN32
    return write(fd, data, dataSize) != -1;
#else
    DWORD written;
    return WriteFile(fd, data, (DWORD) dataSize, &written, nullptr) && written == dataSize;
#endif
}

[[gnu::nonnull(2)]]
static inline bool ffWriteFDBuffer(FFNativeFD fd, const FFstrbuf* content) {
    return ffWriteFDData(fd, content->length, content->chars);
}

[[gnu::nonnull(1, 3)]]
bool ffWriteFileData(const char* fileName, size_t dataSize, const void* data);

[[gnu::nonnull(1, 2)]]
static inline bool ffWriteFileBuffer(const char* fileName, const FFstrbuf* buffer) {
    return ffWriteFileData(fileName, buffer->length, buffer->chars);
}

[[gnu::nonnull(3)]]
static inline ssize_t ffReadFDData(FFNativeFD fd, size_t dataSize, void* data) {
#ifndef _WIN32
    return read(fd, data, dataSize);
#else
    DWORD bytesRead;
    if (!ReadFile(fd, data, (DWORD) dataSize, &bytesRead, nullptr)) {
        return -1;
    }

    return (ssize_t) bytesRead;
#endif
}

[[gnu::nonnull(2)]] bool ffAppendFDBuffer(FFNativeFD fd, FFstrbuf* buffer);

[[gnu::nonnull(1, 3)]] static inline ssize_t ffReadFileData(const char* fileName, size_t dataSize, void* data) {
    FF_AUTO_CLOSE_FD FFNativeFD fd =
#ifndef _WIN32
        open(fileName, O_RDONLY | O_CLOEXEC);
#else
        CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif

    if (!ffIsValidNativeFD(fd)) {
        return -1;
    }

    return ffReadFDData(fd, dataSize, data);
}

[[gnu::nonnull(2, 4)]] static inline ssize_t ffReadFileDataRelative(FFNativeFD dfd, const char* fileName, size_t dataSize, void* data) {
    FF_AUTO_CLOSE_FD FFNativeFD fd = openat(dfd, fileName, O_RDONLY | O_CLOEXEC);
    if (!ffIsValidNativeFD(fd)) {
        return -1;
    }

    return ffReadFDData(fd, dataSize, data);
}

[[gnu::nonnull(1, 2)]] static inline bool ffAppendFileBuffer(const char* fileName, FFstrbuf* buffer) {
    FF_AUTO_CLOSE_FD FFNativeFD fd =
#ifndef _WIN32
        open(fileName, O_RDONLY | O_CLOEXEC);
#else
        CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif

    if (!ffIsValidNativeFD(fd)) {
        return false;
    }

    return ffAppendFDBuffer(fd, buffer);
}

[[gnu::nonnull(2, 3)]] static inline bool ffAppendFileBufferRelative(FFNativeFD dfd, const char* fileName, FFstrbuf* buffer) {
    FF_AUTO_CLOSE_FD FFNativeFD fd = openat(dfd, fileName, O_RDONLY | O_CLOEXEC);
    if (!ffIsValidNativeFD(fd)) {
        return false;
    }

    return ffAppendFDBuffer(fd, buffer);
}

[[gnu::nonnull(2)]] static inline bool ffReadFDBuffer(FFNativeFD fd, FFstrbuf* buffer) {
    ffStrbufClear(buffer);
    return ffAppendFDBuffer(fd, buffer);
}

[[gnu::nonnull(1, 2)]] static inline bool ffReadFileBuffer(const char* fileName, FFstrbuf* buffer) {
    ffStrbufClear(buffer);
    return ffAppendFileBuffer(fileName, buffer);
}

[[gnu::nonnull(2, 3)]] static inline bool ffReadFileBufferRelative(FFNativeFD dfd, const char* fileName, FFstrbuf* buffer) {
    ffStrbufClear(buffer);
    return ffAppendFileBufferRelative(dfd, fileName, buffer);
}

typedef enum FFPathType: uint8_t {
    FF_PATHTYPE_FILE = 1 << 0,
    FF_PATHTYPE_DIRECTORY = 1 << 1,
    FF_PATHTYPE_ANY = FF_PATHTYPE_FILE | FF_PATHTYPE_DIRECTORY,
} FFPathType;

[[gnu::nonnull(1)]] static inline bool ffPathExists(const char* path, FFPathType pathType) {
#ifdef _WIN32

    wchar_t wPath[MAX_PATH];
    if (!NT_SUCCESS(RtlUTF8ToUnicodeN(wPath, (ULONG) sizeof(wPath), nullptr, path, (ULONG) strlen(path) + 1))) {
        return false;
    }

    DWORD attr = GetFileAttributesW(wPath);

    if (attr == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    if (pathType & FF_PATHTYPE_FILE && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }

    if (pathType & FF_PATHTYPE_DIRECTORY && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }

#else

    if (pathType == FF_PATHTYPE_ANY) {
        // Zero overhead
        return access(path, F_OK) == 0;
    } else {
        struct stat fileStat;
        if (stat(path, &fileStat) != 0) {
            return false;
        }

        unsigned int mode = fileStat.st_mode & S_IFMT;

        if (pathType & FF_PATHTYPE_FILE && mode != S_IFDIR) {
            return true;
        }

        if (pathType & FF_PATHTYPE_DIRECTORY && mode == S_IFDIR) {
            return true;
        }
    }

#endif

    return false;
}

[[gnu::nonnull(1, 2)]] bool ffPathExpandEnv(const char* in, FFstrbuf* out);

#define FF_IO_TERM_RESP_WAIT_MS 100 // #554

[[gnu::format(scanf, 3, 4), gnu::nonnull(1, 3)]] const char* ffGetTerminalResponse(const char* request, int nParams, const char* format, ...);

// Not thread safe!
bool ffSuppressIO(bool suppress);

static inline void ffUnsuppressIO(bool* suppressed) {
    if (!*suppressed) {
        return;
    }
    ffSuppressIO(false);
    *suppressed = false;
}

#define FF_SUPPRESS_IO() [[maybe_unused, gnu::cleanup(ffUnsuppressIO)]] bool io_suppressed__ = ffSuppressIO(true)

void ffListFilesRecursively(const char* path, bool pretty);

[[gnu::nonnull(1), gnu::always_inline]] static inline void wrapFclose(FILE** pfile) {
    assert(pfile);
    if (*pfile) {
        fclose(*pfile);
    }
}
#define FF_AUTO_CLOSE_FILE [[gnu::cleanup(wrapFclose)]]

[[gnu::nonnull(1), gnu::always_inline]]
#ifndef _WIN32
static inline void wrapClosedir(DIR** pdir) {
    assert(pdir);
    if (*pdir) {
        closedir(*pdir);
    }
}
#else
static inline void wrapClosedir(HANDLE* pdir) {
    assert(pdir);
    if (*pdir) {
        FindClose(*pdir);
    }
}
#endif
#define FF_AUTO_CLOSE_DIR [[gnu::cleanup(wrapClosedir)]]

[[gnu::nonnull(1, 2, 3)]] static inline bool ffSearchUserConfigFile(const FFlist* configDirs, const char* fileSubpath, FFstrbuf* result) {
    // configDirs is a list of FFstrbufs include the trailing slash
    FF_LIST_FOR_EACH (FFstrbuf, dir, *configDirs) {
        ffStrbufClear(result);
        ffStrbufAppend(result, dir);
        ffStrbufAppendS(result, fileSubpath);
        if (ffPathExists(result->chars, FF_PATHTYPE_FILE)) {
            return true;
        }
    }

    return false;
}

FFNativeFD ffGetNullFD(void);
bool ffRemoveFile(const char* fileName);
