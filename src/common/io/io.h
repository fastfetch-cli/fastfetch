#pragma once

#include "util/FFstrbuf.h"
#include "util/FFlist.h"

#ifdef _WIN32
    #include <fileapi.h>
    #include <handleapi.h>
    #include <io.h>
    typedef HANDLE FFNativeFD;
    #define FF_INVALID_FD INVALID_HANDLE_VALUE
#else
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #include <errno.h>
    #include <limits.h>
    typedef int FFNativeFD;
    #define FF_INVALID_FD (-1)
    // procfs's file can be changed between read calls such as /proc/meminfo and /proc/uptime.
    // one safe way to read correct data is reading the whole file in a single read syscall
    #define PROC_FILE_BUFFSIZ (32 * 1024)
#endif

static inline FFNativeFD FFUnixFD2NativeFD(int unixfd)
{
    #ifndef _WIN32
        return unixfd;
    #else
        return (FFNativeFD) _get_osfhandle(unixfd);
    #endif
}

FF_C_NONNULL(3)
static inline bool ffWriteFDData(FFNativeFD fd, size_t dataSize, const void* data)
{
    #ifndef _WIN32
        return write(fd, data, dataSize) != -1;
    #else
        DWORD written;
        return WriteFile(fd, data, (DWORD) dataSize, &written, NULL) && written == dataSize;
    #endif
}

FF_C_NONNULL(2)
static inline bool ffWriteFDBuffer(FFNativeFD fd, const FFstrbuf* content)
{
    return ffWriteFDData(fd, content->length, content->chars);
}

FF_C_NONNULL(1, 3)
bool ffWriteFileData(const char* fileName, size_t dataSize, const void* data);

FF_C_NONNULL(1, 2)
static inline bool ffWriteFileBuffer(const char* fileName, const FFstrbuf* buffer)
{
    return ffWriteFileData(fileName, buffer->length, buffer->chars);
}

FF_C_NONNULL(3)
static inline ssize_t ffReadFDData(FFNativeFD fd, size_t dataSize, void* data)
{
    #ifndef _WIN32
        return read(fd, data, dataSize);
    #else
        DWORD bytesRead;
        if(!ReadFile(fd, data, (DWORD)dataSize, &bytesRead, NULL))
            return -1;

        return (ssize_t)bytesRead;
    #endif
}

FF_C_NONNULL(1, 3)
ssize_t ffReadFileData(const char* fileName, size_t dataSize, void* data);
FF_C_NONNULL(2, 4)
ssize_t ffReadFileDataRelative(FFNativeFD dfd, const char* fileName, size_t dataSize, void* data);

FF_C_NONNULL(2)
bool ffAppendFDBuffer(FFNativeFD fd, FFstrbuf* buffer);
FF_C_NONNULL(1, 2)
bool ffAppendFileBuffer(const char* fileName, FFstrbuf* buffer);
FF_C_NONNULL(2, 3)
bool ffAppendFileBufferRelative(FFNativeFD dfd, const char* fileName, FFstrbuf* buffer);

FF_C_NONNULL(1, 2)
static inline bool ffReadFileBuffer(const char* fileName, FFstrbuf* buffer)
{
    ffStrbufClear(buffer);
    return ffAppendFileBuffer(fileName, buffer);
}

FF_C_NONNULL(2, 3)
static inline bool ffReadFileBufferRelative(FFNativeFD dfd, const char* fileName, FFstrbuf* buffer)
{
    ffStrbufClear(buffer);
    return ffAppendFileBufferRelative(dfd, fileName, buffer);
}

typedef enum __attribute__((__packed__)) FFPathType
{
    FF_PATHTYPE_FILE = 1 << 0,
    FF_PATHTYPE_DIRECTORY = 1 << 1,
    FF_PATHTYPE_ANY = FF_PATHTYPE_FILE | FF_PATHTYPE_DIRECTORY,
    FF_PATHTYPE_FORCE_UNSIGNED = UINT8_MAX,
} FFPathType;

FF_C_NONNULL(1)
static inline bool ffPathExists(const char* path, FFPathType pathType)
{
    #ifdef _WIN32

    DWORD attr = GetFileAttributesA(path);

    if(attr == INVALID_FILE_ATTRIBUTES)
        return false;

    if(pathType & FF_PATHTYPE_FILE && !(attr & FILE_ATTRIBUTE_DIRECTORY))
        return true;

    if(pathType & FF_PATHTYPE_DIRECTORY && (attr & FILE_ATTRIBUTE_DIRECTORY))
        return true;

    #else

    if (pathType == FF_PATHTYPE_ANY)
    {
        // Zero overhead
        return access(path, F_OK) == 0;
    }
    else
    {
        struct stat fileStat;
        if(stat(path, &fileStat) != 0)
            return false;

        unsigned int mode = fileStat.st_mode & S_IFMT;

        if(pathType & FF_PATHTYPE_FILE && mode != S_IFDIR)
            return true;

        if(pathType & FF_PATHTYPE_DIRECTORY && mode == S_IFDIR)
            return true;
    }

    #endif

    return false;
}

FF_C_NONNULL(1, 2)
bool ffPathExpandEnv(const char* in, FFstrbuf* out);

#define FF_IO_TERM_RESP_WAIT_MS 100 // #554

FF_C_SCANF(3, 4)
FF_C_NONNULL(1, 3)
const char* ffGetTerminalResponse(const char* request, int nParams, const char* format, ...);

// Not thread safe!
bool ffSuppressIO(bool suppress);

static inline void ffUnsuppressIO(bool* suppressed)
{
    if (!*suppressed) return;
    ffSuppressIO(false);
    *suppressed = false;
}

#define FF_SUPPRESS_IO() bool __attribute__((__cleanup__(ffUnsuppressIO), __unused__)) io_suppressed__ = ffSuppressIO(true)

void ffListFilesRecursively(const char* path, bool pretty);

FF_C_NONNULL(1)
static inline bool wrapClose(FFNativeFD* pfd)
{
    assert(pfd);

    #ifndef WIN32
        if (*pfd < 0)
            return false;
        close(*pfd);
    #else
        // https://devblogs.microsoft.com/oldnewthing/20040302-00/?p=40443
        if (*pfd == NULL || *pfd == INVALID_HANDLE_VALUE)
            return false;
        CloseHandle(*pfd);
    #endif

    return true;
}
#define FF_AUTO_CLOSE_FD __attribute__((__cleanup__(wrapClose)))

FF_C_NONNULL(1)
static inline bool wrapFclose(FILE** pfile)
{
    assert(pfile);
    if (!*pfile)
        return false;
    fclose(*pfile);
    return true;
}
#define FF_AUTO_CLOSE_FILE __attribute__((__cleanup__(wrapFclose)))

FF_C_NONNULL(1)
#ifndef _WIN32
static inline bool wrapClosedir(DIR** pdir)
{
    assert(pdir);
    if (!*pdir)
        return false;
    closedir(*pdir);
    return true;
}
#else
static inline bool wrapClosedir(HANDLE* pdir)
{
    assert(pdir);
    if (!*pdir)
        return false;
    FindClose(*pdir);
    return true;
}
#endif
#define FF_AUTO_CLOSE_DIR __attribute__((__cleanup__(wrapClosedir)))

FF_C_NONNULL(1, 2, 3)
static inline bool ffSearchUserConfigFile(const FFlist* configDirs, const char* fileSubpath, FFstrbuf* result)
{
    // configDirs is a list of FFstrbufs include the trailing slash
    FF_LIST_FOR_EACH(FFstrbuf, dir, *configDirs)
    {
        ffStrbufClear(result);
        ffStrbufAppend(result, dir);
        ffStrbufAppendS(result, fileSubpath);
        if (ffPathExists(result->chars, FF_PATHTYPE_FILE))
            return true;
    }

    return false;
}

FFNativeFD ffGetNullFD(void);

#ifdef _WIN32
// Only O_RDONLY is supported
HANDLE openat(HANDLE dfd, const char* fileName, bool directory);
#endif
