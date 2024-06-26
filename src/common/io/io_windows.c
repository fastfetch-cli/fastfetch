#include "io.h"
#include "fastfetch.h"
#include "util/stringUtils.h"

#include <windows.h>

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
    ffStrbufEnsureFixedLengthFree(buffer, length);
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
    (void) suppress; //Not implemented.
    return false;
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

const char* ffGetTerminalResponse(const char* request, const char* format, ...)
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
        WriteFile(hOutput, "TEST\n", 5, &bytes, NULL);
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

    char buffer[512];
    DWORD bytes = 0;
    ReadFile(hInput, buffer, sizeof(buffer) - 1, &bytes, NULL);

    SetConsoleMode(hInput, inputMode);

    if(bytes <= 0)
        return "ReadFile() failed";

    buffer[bytes] = '\0';

    va_list args;
    va_start(args, format);
    vsscanf(buffer, format, args);
    va_end(args);

    return NULL;
}
