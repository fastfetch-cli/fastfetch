#include "fastfetch.h"
#include "common/io.h"
#include "common/stringUtils.h"
#include "common/windows/nt.h"
#include "common/windows/unicode.h"

#include <windows.h>

static bool createSubfolders(wchar_t* fileName)
{
    HANDLE hRoot = ffGetPeb()->ProcessParameters->CurrentDirectory.Handle;
    bool closeRoot = false;
    wchar_t* ptr = fileName;

    // Absolute drive path: C:\...
    if (ffCharIsEnglishAlphabet((char)ptr[0]) && ptr[1] == L':' && ptr[2] == L'\\')
    {
        wchar_t saved = ptr[3];
        ptr[3] = L'\0';

        hRoot = CreateFileW(
            fileName,
            FILE_LIST_DIRECTORY | FILE_TRAVERSE | SYNCHRONIZE,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_FLAG_BACKUP_SEMANTICS,
            NULL
        );

        ptr[3] = saved;
        if (hRoot == INVALID_HANDLE_VALUE)
            return false;

        closeRoot = true;
        ptr += 3; // skip "C:\"
    }
    // UNC path: \\server\share\...
    else if (ptr[0] == L'\\' && ptr[1] == L'\\')
    {
        wchar_t* serverEnd = wcschr(ptr + 2, L'\\');
        if (serverEnd == NULL)
            return false;

        wchar_t* shareEnd = wcschr(serverEnd + 1, L'\\');
        if (shareEnd == NULL)
            return true; // no parent subfolder exists before file name

        wchar_t saved = *shareEnd;
        *shareEnd = L'\0';

        hRoot = CreateFileW(
            fileName,
            FILE_LIST_DIRECTORY | FILE_TRAVERSE | SYNCHRONIZE,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_FLAG_BACKUP_SEMANTICS,
            NULL
        );

        *shareEnd = saved;
        if (hRoot == INVALID_HANDLE_VALUE)
            return false;

        closeRoot = true;
        ptr = shareEnd + 1; // first component under share
    }
    // Rooted path on current drive: \foo\bar
    else if (ptr[0] == L'\\')
    {
        UNICODE_STRING* dosPath = &ffGetPeb()->ProcessParameters->CurrentDirectory.DosPath;
        wchar_t driveRoot[] = { dosPath->Buffer[0], L':', L'\\', L'\0' };
        hRoot = CreateFileW(
            driveRoot,
            FILE_LIST_DIRECTORY | FILE_TRAVERSE | SYNCHRONIZE,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_FLAG_BACKUP_SEMANTICS,
            NULL
        );
        if (hRoot == INVALID_HANDLE_VALUE)
            return false;
        closeRoot = true;
        ptr++; // skip leading '\'
    }

    while (true)
    {
        wchar_t* token = wcschr(ptr, L'\\');
        if (token == NULL)
            break;

        // Skip empty path segments caused by duplicated '\'
        if (token == ptr)
        {
            ptr = token + 1;
            continue;
        }

        HANDLE hNew = INVALID_HANDLE_VALUE;
        IO_STATUS_BLOCK iosb = {};

        NTSTATUS status = NtCreateFile(
            &hNew,
            FILE_LIST_DIRECTORY | FILE_TRAVERSE | SYNCHRONIZE,
            &(OBJECT_ATTRIBUTES) {
                .Length = sizeof(OBJECT_ATTRIBUTES),
                .RootDirectory = hRoot,
                .ObjectName = &(UNICODE_STRING) {
                    .Buffer = ptr,
                    .Length = (USHORT)((USHORT)(token - ptr) * sizeof(wchar_t)),
                    .MaximumLength = (USHORT)((USHORT)(token - ptr) * sizeof(wchar_t)),
                },
                .Attributes = OBJ_CASE_INSENSITIVE,
            },
            &iosb,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            FILE_OPEN_IF,
            FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
            NULL,
            0
        );

        if (!NT_SUCCESS(status))
        {
            if (closeRoot && hRoot != INVALID_HANDLE_VALUE)
                NtClose(hRoot);
            return false;
        }

        if (closeRoot && hRoot != INVALID_HANDLE_VALUE)
            NtClose(hRoot);
        hRoot = hNew;
        closeRoot = true;

        ptr = token + 1;
    }

    if (closeRoot && hRoot != INVALID_HANDLE_VALUE)
        NtClose(hRoot);

    return true;
}

bool ffWriteFileData(const char* fileName, size_t dataSize, const void* data)
{
    wchar_t fileNameW[MAX_PATH];
    ULONG len = 0;
    if (!NT_SUCCESS(RtlUTF8ToUnicodeN(fileNameW, (ULONG) sizeof(fileNameW), &len, fileName, (ULONG)strlen(fileName) + 1)))
        return false;

    for (ULONG i = 0; i < len / sizeof(wchar_t); ++i)
    {
        if (fileNameW[i] == L'/')
            fileNameW[i] = L'\\';
    }

    HANDLE FF_AUTO_CLOSE_FD handle = CreateFileW(fileNameW, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_PATH_NOT_FOUND)
        {
            if (!createSubfolders(fileNameW))
                return false;
            handle = CreateFileW(fileNameW, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
    FILE_STANDARD_INFORMATION fileInfo;
    IO_STATUS_BLOCK iosb;
    if(!NT_SUCCESS(NtQueryInformationFile(handle, &iosb, &fileInfo, sizeof(fileInfo), FileStandardInformation)))
        fileInfo.EndOfFile.QuadPart = 0;

    if (fileInfo.EndOfFile.QuadPart > 0)
        readWithLength(handle, buffer, (uint32_t)fileInfo.EndOfFile.QuadPart);
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

HANDLE openatW(HANDLE dfd, const wchar_t* fileName, uint16_t fileNameLen, bool directory)
{
    assert(fileNameLen <= 0x7FFF);

    HANDLE hFile;
    IO_STATUS_BLOCK iosb = {};
    if(!NT_SUCCESS(NtOpenFile(&hFile,
        (directory ? FILE_LIST_DIRECTORY | FILE_TRAVERSE : FILE_READ_DATA | FILE_READ_EA) | FILE_READ_ATTRIBUTES | SYNCHRONIZE, &(OBJECT_ATTRIBUTES) {
            .Length = sizeof(OBJECT_ATTRIBUTES),
            .RootDirectory = dfd,
            .ObjectName = &(UNICODE_STRING) {
                .Buffer = (PWSTR) fileName,
                .Length = fileNameLen * (USHORT) sizeof(wchar_t),
                .MaximumLength = (fileNameLen + 1) * (USHORT) sizeof(wchar_t),
            },
            .Attributes = OBJ_CASE_INSENSITIVE,
        },
        &iosb,
        FILE_SHARE_READ | (directory ? FILE_SHARE_WRITE | FILE_SHARE_DELETE : 0),
        FILE_SYNCHRONOUS_IO_NONALERT | (directory ? FILE_DIRECTORY_FILE : FILE_NON_DIRECTORY_FILE)
    )))
        return INVALID_HANDLE_VALUE;

    return hFile;
}

HANDLE openat(HANDLE dfd, const char* fileName, bool directory)
{
    wchar_t fileNameW[MAX_PATH];
    ULONG len;
    if (!NT_SUCCESS(RtlUTF8ToUnicodeN(fileNameW, (ULONG) sizeof(fileNameW), &len, fileName, (ULONG)strlen(fileName) + 1)))
        return INVALID_HANDLE_VALUE;
    // Implies `fileNameW[len] = L'\0';` and `len` includes the null terminator
    len /= sizeof(wchar_t); // convert from bytes to characters

    for (uint32_t i = 0; i < len - 1; ++i)
    {
        if (fileNameW[i] == L'/')
            fileNameW[i] = L'\\';
    }

    return openatW(dfd, fileNameW, (uint16_t)(len - 1), directory);
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
    if (in[0] == '~') {
        if ((in[1] == '/' || in[1] == '\\' || in[1] == '\0') && !ffStrContainsC(in, '%')) {
            ffStrbufSet(out, &instance.state.platform.homeDir);
            ffStrbufAppendS(out, in + 1);
            return true;
        }
    }

    wchar_t pathInW[MAX_PATH], pathOutW[MAX_PATH];
    ULONG len = (ULONG) strlen(in);
    if (!NT_SUCCESS(RtlUTF8ToUnicodeN(pathInW, (ULONG) sizeof(pathInW), &len, in, len)))
        return false;
    len /= sizeof(wchar_t); // convert from bytes to characters

    size_t outLen; // in characters, including null terminator
    if (!NT_SUCCESS(RtlExpandEnvironmentStrings(NULL, pathInW, len, pathOutW, ARRAY_SIZE(pathOutW), &outLen)))
        return false;

    ffStrbufSetNWS(out, (uint32_t) outLen - 1, pathOutW);
    return true;
}

bool ffSuppressIO(bool suppress)
{
    #ifndef NDEBUG
    if (instance.config.display.debugMode)
        return false;
    #endif

    static bool init = false;
    static HANDLE hOrigOut = INVALID_HANDLE_VALUE;
    static HANDLE hOrigErr = INVALID_HANDLE_VALUE;
    HANDLE hNullFile = ffGetNullFD();
    static int fOrigOut = -1;
    static int fOrigErr = -1;
    static int fNullFile = -1;

    if (!init)
    {
        if(!suppress)
            return true;

        hOrigOut = GetStdHandle(STD_OUTPUT_HANDLE);
        hOrigErr = GetStdHandle(STD_ERROR_HANDLE);
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
        if (NtWaitForSingleObject(hInput, TRUE, &(LARGE_INTEGER) { .QuadPart = (int64_t) FF_IO_TERM_RESP_WAIT_MS * -10000 }) != STATUS_WAIT_0)
        {
            SetConsoleMode(hInput, inputMode);
            return "NtWaitForSingleObject() failed or timeout";
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
        int ret = vsscanf(buffer, format, cargs);
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

FFNativeFD ffGetNullFD(void)
{
    static FFNativeFD hNullFile = INVALID_HANDLE_VALUE;
    if (hNullFile != INVALID_HANDLE_VALUE)
        return hNullFile;
    hNullFile = CreateFileW(
        L"NUL",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE,
        0,
        OPEN_EXISTING,
        0,
        &(SECURITY_ATTRIBUTES){
            .nLength = sizeof(SECURITY_ATTRIBUTES),
            .lpSecurityDescriptor = NULL,
            .bInheritHandle = TRUE,
        });
    return hNullFile;
}

bool ffRemoveFile(const char* fileName)
{
    return DeleteFileA(fileName) != FALSE;
}
