#include "fastfetch.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <termios.h>
#include <poll.h>

bool ffWriteFDContent(int fd, const FFstrbuf* content)
{
    return write(fd, content->chars, content->length) != -1;
}

static void createSubfolders(const char* fileName)
{
    FFstrbuf path;
    ffStrbufInit(&path);

    while(*fileName != '\0')
    {
        ffStrbufAppendC(&path, *fileName);
        if(*fileName == '/')
            mkdir(path.chars, S_IRWXU | S_IRGRP | S_IROTH);
        ++fileName;
    }

    ffStrbufDestroy(&path);
}

bool ffWriteFileContent(const char* fileName, const FFstrbuf* content)
{
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

    bool ret = ffWriteFDContent(fd, content);

    close(fd);

    return ret;
}

void ffAppendFDContent(int fd, FFstrbuf* buffer)
{
    ssize_t readed = 0;

    ffStrbufEnsureFree(buffer, 31); // 32 - 1 for the null terminator
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
}

bool ffAppendFileContent(const char* fileName, FFstrbuf* buffer)
{
    int fd = open(fileName, O_RDONLY);
    if(fd == -1)
        return false;

    ffAppendFDContent(fd, buffer);

    close(fd);
    return true;
}

bool ffGetFileContent(const char* fileName, FFstrbuf* buffer)
{
    ffStrbufClear(buffer);
    return ffAppendFileContent(fileName, buffer);
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
}
