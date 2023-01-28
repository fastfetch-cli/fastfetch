#pragma once

#ifndef FF_INCLUDED_common_io_io
#define FF_INCLUDED_common_io_io

#include "fastfetch.h"

#ifdef _WIN32
    #include <fileapi.h>
    typedef HANDLE FFNativeFD;
#else
    #include <unistd.h>
    typedef int FFNativeFD;
#endif

static inline bool ffWriteFDBuffer(FFNativeFD fd, const FFstrbuf* content)
{
    #ifndef _WIN32
        return write(fd, content->chars, content->length) != -1;
    #else
        DWORD written;
        return WriteFile(fd, content->chars, content->length, &written, NULL) && written == content->length;
    #endif
}

bool ffWriteFileData(const char* fileName, size_t dataSize, const void* data);

static inline bool ffWriteFileBuffer(const char* fileName, const FFstrbuf* buffer)
{
    return ffWriteFileData(fileName, buffer->length, buffer->chars);
}

bool ffAppendFDBuffer(FFNativeFD fd, FFstrbuf* buffer);
ssize_t ffReadFileData(const char* fileName, size_t dataSize, void* data);
bool ffAppendFileBuffer(const char* fileName, FFstrbuf* buffer);

static inline bool ffReadFileBuffer(const char* fileName, FFstrbuf* buffer)
{
    ffStrbufClear(buffer);
    return ffAppendFileBuffer(fileName, buffer);
}

//Bit flags, combine with |
typedef enum FFPathType
{
    FF_PATHTYPE_REGULAR = 1,
    FF_PATHTYPE_LINK = 2,
    FF_PATHTYPE_DIRECTORY = 4
} FFPathType;

#define FF_PATHTYPE_FILE (FF_PATHTYPE_REGULAR | FF_PATHTYPE_LINK)
#define FF_PATHTYPE_ANY (FF_PATHTYPE_FILE | FF_PATHTYPE_DIRECTORY)

bool ffPathExists(const char* path, FFPathType pathType);

#ifndef _WIN32
    FF_C_SCANF(2, 3)
    void ffGetTerminalResponse(const char* request, const char* format, ...);
#endif

// Not thread safe!
void ffSuppressIO(bool suppress);

#endif // FF_INCLUDED_common_io_io
