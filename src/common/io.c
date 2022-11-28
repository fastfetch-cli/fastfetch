#include "fastfetch.h"
#include "common/io.h"

#include <unistd.h>
#include <fcntl.h>

#ifdef _WIN32
    #include <fileapi.h>
#else
    #include <termios.h>
    #include <poll.h>
#endif

static void createSubfolders(const char* fileName)
{
    FFstrbuf path;
    ffStrbufInit(&path);

    while(*fileName != '\0')
    {
        ffStrbufAppendC(&path, *fileName);
        if(*fileName == '/')
        {
            mkdir(path.chars
                #ifndef WIN32
                , S_IRWXU | S_IRGRP | S_IROTH
                #endif
            );
        }
        ++fileName;
    }

    ffStrbufDestroy(&path);
}

bool ffWriteFDBuffer(int fd, const FFstrbuf* content)
{
    return write(fd, content->chars, content->length) != -1;
}

bool ffWriteFileData(const char* fileName, size_t dataSize, const void* data)
{
    #ifdef _WIN32
    HANDLE handle = CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(handle == INVALID_HANDLE_VALUE)
    {
        createSubfolders(fileName);
        handle = CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(handle == INVALID_HANDLE_VALUE)
            return false;
    }

    DWORD written;
    bool ret = !!WriteFile(handle, data, (DWORD)dataSize, &written, NULL);

    CloseHandle(handle);
    #else
    int openFlagsModes = O_WRONLY | O_CREAT | O_TRUNC;
    int openFlagsRights = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    int fd = open(fileName, openFlagsModes, openFlagsRights);
    if(fd == -1)
    {
        createSubfolders(fileName);
        fd = open(fileName, openFlagsModes, openFlagsRights);
        if(fd == -1)
            return false;
    }

    bool ret = write(fd, data, dataSize) != -1;

    close(fd);
    #endif

    return ret;
}

bool ffWriteFileBuffer(const char* fileName, const FFstrbuf* buffer)
{
    return ffWriteFileData(fileName, buffer->length, buffer->chars);
}

#ifdef _WIN32

bool ffAppendHandleBuffer(HANDLE handle, FFstrbuf* buffer)
{
    DWORD readed = 0;

    LARGE_INTEGER fileSize;
    if(!GetFileSizeEx(handle, &fileSize))
        fileSize.QuadPart = 0;

    ffStrbufEnsureFree(buffer, fileSize.QuadPart > 0 ? (uint32_t)fileSize.QuadPart : 31);
    uint32_t free = ffStrbufGetFree(buffer);

    bool success;
    while(
        (success = !!ReadFile(handle, buffer->chars + buffer->length, free, &readed, NULL)) &&
        (uint32_t) readed == free
    ) {
        buffer->length += (uint32_t) readed;
        ffStrbufEnsureFree(buffer, buffer->allocated - 1); // Doubles capacity every round. -1 for the null byte.
        free = ffStrbufGetFree(buffer);
    }

    if(readed > 0)
        buffer->length += (uint32_t) readed;

    buffer->chars[buffer->length] = '\0';

    ffStrbufTrimRight(buffer, '\n');
    ffStrbufTrimRight(buffer, ' ');

    return success;
}

#endif

bool ffAppendFDBuffer(int fd, FFstrbuf* buffer)
{
    ssize_t readed = 0;

    struct stat fileInfo;
    if(fstat(fd, &fileInfo) != 0)
        return false;

    ffStrbufEnsureFree(buffer, fileInfo.st_size > 0 ? (uint32_t)fileInfo.st_size : 31);
    uint32_t free = ffStrbufGetFree(buffer);

    while(
        (readed = read(fd, buffer->chars + buffer->length, free)) > 0 &&
        (uint32_t) readed == free
    ) {
        buffer->length += (uint32_t) readed;
        ffStrbufEnsureFree(buffer, buffer->allocated - 1); // Doubles capacity every round. -1 for the null byte.
        free = ffStrbufGetFree(buffer);
    }

    // In case of failure, read returns -1. We don't want to substract the length of the buffer.
    if(readed > 0)
        buffer->length += (uint32_t) readed;

    buffer->chars[buffer->length] = '\0';

    ffStrbufTrimRight(buffer, '\n');
    ffStrbufTrimRight(buffer, ' ');

    return readed >= 0;
}

ssize_t ffReadFileData(const char* fileName, size_t dataSize, void* data)
{
    #ifdef _WIN32
    HANDLE handle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(handle == INVALID_HANDLE_VALUE)
        return false;

    DWORD readed;
    if(!ReadFile(handle, data, (DWORD)dataSize, &readed, NULL))
        return -1;

    return (ssize_t)readed;
    #else
    int fd = open(fileName, O_RDONLY);
    if(fd == -1)
        return -1;

    ssize_t readed = read(fd, data, dataSize);

    close(fd);

    return readed;
    #endif
}

bool ffAppendFileBuffer(const char* fileName, FFstrbuf* buffer)
{
    #ifdef _WIN32
    HANDLE handle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(handle == INVALID_HANDLE_VALUE)
        return false;

    bool ret = ffAppendHandleBuffer(handle, buffer);

    CloseHandle(handle);
    #else
    int fd = open(fileName, O_RDONLY);
    if(fd == -1)
        return false;

    bool ret = ffAppendFDBuffer(fd, buffer);

    close(fd);
    #endif

    return ret;
}

bool ffReadFileBuffer(const char* fileName, FFstrbuf* buffer)
{
    ffStrbufClear(buffer);
    return ffAppendFileBuffer(fileName, buffer);
}

// Not thread safe!
void ffSuppressIO(bool suppress)
{
    static bool init = false;
    static int origOut = -1;
    static int origErr = -1;
    static int nullFile = -1;

    if(!init)
    {
        if(!suppress)
            return;

        origOut = dup(STDOUT_FILENO);
        origErr = dup(STDERR_FILENO);
        nullFile = open("/dev/null", O_WRONLY);
        init = true;
    }

    if(nullFile == -1)
        return;

    fflush(stdout);
    fflush(stderr);

    dup2(suppress ? nullFile : origOut, STDOUT_FILENO);
    dup2(suppress ? nullFile : origErr, STDERR_FILENO);
}

bool ffFileExists(const char* fileName, mode_t mode)
{
    struct stat fileStat;
    return stat(fileName, &fileStat) == 0 && ((fileStat.st_mode & S_IFMT) == mode);
}

void ffGetTerminalResponse(const char* request, const char* format, ...)
{
    #ifndef WIN32
    struct termios oldTerm, newTerm;
    if(tcgetattr(STDIN_FILENO, &oldTerm) == -1)
        return;

    newTerm = oldTerm;
    newTerm.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    if(tcsetattr(STDIN_FILENO, TCSANOW, &newTerm) == -1)
        return;

    fputs(request, stdout);
    fflush(stdout);

    struct pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    pfd.revents = 0;

    //Give the terminal 35ms to respond
    if(poll(&pfd, 1, 35) <= 0)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldTerm);
        return;
    }

    char buffer[512];
    ssize_t readed = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldTerm);

    if(readed <= 0)
        return;

    buffer[readed] = '\0';

    va_list args;
    va_start(args, format);
    vsscanf(buffer, format, args);
    va_end(args);
    #else
    //Unimplemented
    FF_UNUSED(request, format);
    #endif
}
