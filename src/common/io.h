#pragma once

#ifndef FF_INCLUDED_common_io
#define FF_INCLUDED_common_io

#include "fastfetch.h"

bool ffWriteFDBuffer(int fd, const FFstrbuf* content);
bool ffWriteFileData(const char* fileName, size_t dataSize, const void* data);
bool ffWriteFileBuffer(const char* fileName, const FFstrbuf* buffer);

bool ffAppendFDBuffer(int fd, FFstrbuf* buffer);
ssize_t ffReadFileData(const char* fileName, size_t dataSize, void* data);
bool ffAppendFileBuffer(const char* fileName, FFstrbuf* buffer);
bool ffReadFileBuffer(const char* fileName, FFstrbuf* buffer);

#ifdef _WIN32
bool ffAppendHandleBuffer(HANDLE handle, FFstrbuf* buffer);
#endif

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

// Not thread safe!
void ffSuppressIO(bool suppress);

FF_C_SCANF(2, 3) void ffGetTerminalResponse(const char* request, const char* format, ...);

#endif
