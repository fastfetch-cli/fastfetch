#include "io.h"
#include "fastfetch.h"
#include "util/stringUtils.h"

#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>

static void createSubfolders(const char* fileName)
{
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
    char *token = NULL;
    while((token = strchr(fileName, '/')) != NULL)
    {
        ffStrbufAppendNS(&path, (uint32_t)(token - fileName + 1), fileName);
        CreateDirectoryA(path.chars, NULL);
        fileName = token + 1;
    }
}

bool ffWriteFileData(const char* fileName, size_t dataSize, const void* data)
{
    HANDLE FF_AUTO_CLOSE_FD handle = CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_PATH_NOT_FOUND)
        {
            createSubfolders(fileName);
            handle = CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (handle == INVALID_HANDLE_VALUE)
                return false;
        }
        else
            return false;
    }

    DWORD written;
    return !!WriteFile(handle, data, (DWORD)dataSize, &written, NULL);
}

static inline void readWithLength(HANDLE handle, FFstrbuf* buffer, uint32_t length)
{
    ffStrbufEnsureFree(buffer, length);
    DWORD bytesRead = 0;
    while(
        length > 0 &&
        ReadFile(handle, buffer->chars + buffer->length, length, &bytesRead, NULL) != FALSE &&
        bytesRead > 0
    ) {
        buffer->length += (uint32_t) bytesRead;
        length -= (uint32_t) bytesRead;
    }
}

static inline void readUntilEOF(HANDLE handle, FFstrbuf* buffer)
{
    ffStrbufEnsureFree(buffer, 31);
    uint32_t available = ffStrbufGetFree(buffer);
    DWORD bytesRead = 0;
    while(
        ReadFile(handle, buffer->chars + buffer->length, available, &bytesRead, NULL) != FALSE &&
        bytesRead > 0
    ) {
        buffer->length += (uint32_t) bytesRead;
        if((uint32_t) bytesRead == available)
            ffStrbufEnsureFree(buffer, buffer->allocated - 1); // Doubles capacity every round. -1 for the null byte.
        available = ffStrbufGetFree(buffer);
    }
}

bool ffAppendFDBuffer(HANDLE handle, FFstrbuf* buffer)
{
    LARGE_INTEGER fileSize;
    if(!GetFileSizeEx(handle, &fileSize))
        fileSize.QuadPart = 0;

    if (fileSize.QuadPart > 0)
        readWithLength(handle, buffer, (uint32_t)fileSize.QuadPart);
    else
        readUntilEOF(handle, buffer);

    buffer->chars[buffer->length] = '\0';

    return buffer->length > 0;
}

ssize_t ffReadFileData(const char* fileName, size_t dataSize, void* data)
{
    HANDLE FF_AUTO_CLOSE_FD handle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(handle == INVALID_HANDLE_VALUE)
        return -1;

    return ffReadFDData(handle, dataSize, data);
}

bool ffAppendFileBuffer(const char* fileName, FFstrbuf* buffer)
{
    HANDLE FF_AUTO_CLOSE_FD handle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(handle == INVALID_HANDLE_VALUE)
        return false;

    return ffAppendFDBuffer(handle, buffer);
}

HANDLE openat(HANDLE dfd, const char* fileName, bool directory)
{
    NTSTATUS ret;
    UNICODE_STRING fileNameW;
    ret = RtlAnsiStringToUnicodeString(&fileNameW, &(ANSI_STRING) {
        .Length = (USHORT) strlen(fileName),
        .Buffer = (PCHAR) fileName
    }, TRUE);
    if (!NT_SUCCESS(ret)) return INVALID_HANDLE_VALUE;

    FF_AUTO_CLOSE_FD HANDLE hFile;
    IO_STATUS_BLOCK iosb = {};
    ret = NtOpenFile(&hFile, FILE_READ_DATA | SYNCHRONIZE, &(OBJECT_ATTRIBUTES) {
        .Length = sizeof(OBJECT_ATTRIBUTES),
        .RootDirectory = dfd,
        .ObjectName = &fileNameW,
    }, &iosb, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT | (directory ? FILE_DIRECTORY_FILE : FILE_NON_DIRECTORY_FILE));
    RtlFreeUnicodeString(&fileNameW);

    if(!NT_SUCCESS(ret) || iosb.Information != FILE_OPENED)
        return INVALID_HANDLE_VALUE;

    return hFile;
}

bool ffAppendFileBufferRelative(HANDLE dfd, const char* fileName, FFstrbuf* buffer)
{
    HANDLE FF_AUTO_CLOSE_FD fd = openat(dfd, fileName, false);
    if(fd == INVALID_HANDLE_VALUE)
        return false;

    return ffAppendFDBuffer(fd, buffer);
}

ssize_t ffReadFileDataRelative(HANDLE dfd, const char* fileName, size_t dataSize, void* data)
{
    HANDLE FF_AUTO_CLOSE_FD fd = openat(dfd, fileName, false);
    if(fd == INVALID_HANDLE_VALUE)
        return -1;

    return ffReadFDData(fd, dataSize, data);
}

bool ffPathExpandEnv(const char* in, FFstrbuf* out)
{
    DWORD length = ExpandEnvironmentStringsA(in, NULL, 0);
    if (length <= 1) return false;

    ffStrbufClear(out);
    ffStrbufEnsureFree(out, (uint32_t)length);
    ExpandEnvironmentStringsA(in, out->chars, length);
    out->length = (uint32_t)length - 1;
    return true;
}

bool ffSuppressIO(bool suppress)
{
    static bool init = false;
    static HANDLE hOrigOut = INVALID_HANDLE_VALUE;
    static HANDLE hOrigErr = INVALID_HANDLE_VALUE;
    static HANDLE hNullFile = INVALID_HANDLE_VALUE;
    static int fOrigOut = -1;
    static int fOrigErr = -1;
    static int fNullFile = -1;

    if (!init)
    {
        if(!suppress)
            return true;

        hOrigOut = GetStdHandle(STD_OUTPUT_HANDLE);
        hOrigErr = GetStdHandle(STD_ERROR_HANDLE);
        hNullFile = CreateFileW(L"NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, NULL);
        fOrigOut = _dup(STDOUT_FILENO);
        fOrigErr = _dup(STDERR_FILENO);
        fNullFile = _open_osfhandle((intptr_t) hNullFile, 0);

        init = true;
    }
    if (hNullFile == INVALID_HANDLE_VALUE || fNullFile == -1)
        return false;

    fflush(stdout);
    fflush(stderr);

    SetStdHandle(STD_OUTPUT_HANDLE, suppress ? hNullFile : hOrigOut);
    SetStdHandle(STD_ERROR_HANDLE, suppress ? hNullFile : hOrigErr);
    _dup2(suppress ? fNullFile : fOrigOut, STDOUT_FILENO);
    _dup2(suppress ? fNullFile : fOrigErr, STDERR_FILENO);

    return true;
}

void listFilesRecursively(uint32_t baseLength, FFstrbuf* folder, uint8_t indentation, const char* folderName, bool pretty)
{
    uint32_t folderLength = folder->length;

    if(pretty && folderName != NULL)
    {
        for(uint8_t i = 0; i < indentation - 1; i++)
            fputs("  | ", stdout);
        printf("%s/\n", folderName);
    }

    ffStrbufAppendC(folder, '*');
    WIN32_FIND_DATAA entry;
    HANDLE hFind = FindFirstFileA(folder->chars, &entry);
    ffStrbufTrimRight(folder, '*');
    if(hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if(ffStrEquals(entry.cFileName, ".") || ffStrEquals(entry.cFileName, ".."))
                continue;

            ffStrbufSubstrBefore(folder, folderLength);
            ffStrbufAppendS(folder, entry.cFileName);
            ffStrbufAppendC(folder, '/');
            listFilesRecursively(baseLength, folder, (uint8_t) (indentation + 1), entry.cFileName, pretty);
            ffStrbufSubstrBefore(folder, folderLength);
            continue;
        }

        if (pretty)
        {
            for(uint8_t i = 0; i < indentation; i++)
                fputs("  | ", stdout);
        }
        else
        {
            fputs(folder->chars + baseLength, stdout);
        }

        puts(entry.cFileName);
    } while (FindNextFileA(hFind, &entry));
    FindClose(hFind);
}

void ffListFilesRecursively(const char* path, bool pretty)
{
    FF_STRBUF_AUTO_DESTROY folder = ffStrbufCreateS(path);
    ffStrbufEnsureEndsWithC(&folder, '/');
    listFilesRecursively(folder.length, &folder, 0, NULL, pretty);
}

const char* ffGetTerminalResponse(const char* request, int nParams, const char* format, ...)
{
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    FF_AUTO_CLOSE_FD HANDLE hConin = INVALID_HANDLE_VALUE;
    DWORD inputMode;
    if (!GetConsoleMode(hInput, &inputMode))
    {
        hConin = CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);
        hInput = hConin;
    }
    SetConsoleMode(hInput, 0);

    FlushConsoleInputBuffer(hInput);

    {
        DWORD bytes = 0;
        HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        FF_AUTO_CLOSE_FD HANDLE hConout = INVALID_HANDLE_VALUE;
        DWORD outputMode;
        if (!GetConsoleMode(hOutput, &outputMode))
        {
            hConout = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, NULL);
            hOutput = hConout;
        }
        WriteFile(hOutput, request, (DWORD) strlen(request), &bytes, NULL);
    }

    while (true)
    {
        if (WaitForSingleObjectEx(hInput, FF_IO_TERM_RESP_WAIT_MS, TRUE) != WAIT_OBJECT_0)
        {
            SetConsoleMode(hInput, inputMode);
            return "WaitForSingleObject() failed or timeout";
        }

        // Ignore all unexpected input events
        INPUT_RECORD record;
        DWORD len = 0;
        if (!PeekConsoleInputW(hInput, &record, 1, &len))
            break;

        if (
            record.EventType == KEY_EVENT &&
            record.Event.KeyEvent.uChar.UnicodeChar != L'\r' &&
            record.Event.KeyEvent.uChar.UnicodeChar != L'\n'
        )
            break;
        else
            ReadConsoleInputW(hInput, &record, 1, &len);
    }

    va_list args;
    va_start(args, format);

    char buffer[1024];
    uint32_t bytesRead = 0;

    while (true)
    {
        DWORD bytes = 0;
        if (!ReadFile(hInput, buffer, sizeof(buffer) - 1, &bytes, NULL) || bytes == 0)
        {
            va_end(args);
            return "ReadFile() failed";
        }

        bytesRead += bytes;
        buffer[bytesRead] = '\0';

        va_list cargs;
        va_copy(cargs, args);
        int ret = vsscanf(buffer, format, args);
        va_end(cargs);

        if (ret <= 0)
        {
            va_end(args);
            return "vsscanf(buffer, format, args) failed";
        }
        if (ret >= nParams)
            break;
    }

    SetConsoleMode(hInput, inputMode);

    va_end(args);

    return NULL;
}
