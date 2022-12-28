#pragma once

#ifndef FF_INCLUDED_common_io
#define FF_INCLUDED_common_io

#include "fastfetch.h"

#include <sys/stat.h> //mode_t, fstat, stat

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

bool ffFileExists(const char* fileName, mode_t mode);

// Not thread safe!
void ffSuppressIO(bool suppress);

FF_C_SCANF(2, 3) void ffGetTerminalResponse(const char* request, const char* format, ...);

#endif
