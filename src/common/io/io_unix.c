#include "io.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <poll.h>
#include <dirent.h>

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

bool ffWriteFileData(const char* fileName, size_t dataSize, const void* data)
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

    bool ret = write(fd, data, dataSize) != -1;

    close(fd);

    return ret;
}

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
    int fd = open(fileName, O_RDONLY);
    if(fd == -1)
        return -1;

    ssize_t readed = ffReadFDData(fd, dataSize, data);

    close(fd);

    return readed;
}

bool ffAppendFileBuffer(const char* fileName, FFstrbuf* buffer)
{
    int fd = open(fileName, O_RDONLY);
    if(fd == -1)
        return false;

    bool ret = ffAppendFDBuffer(fd, buffer);

    close(fd);

    return ret;
}

bool ffPathExists(const char* path, FFPathType type)
{
    struct stat fileStat;
    if(stat(path, &fileStat) != 0)
        return false;

    int mode = fileStat.st_mode & S_IFMT;

    if(type & FF_PATHTYPE_REGULAR && mode == S_IFREG)
        return true;

    if(type & FF_PATHTYPE_DIRECTORY && mode == S_IFDIR)
        return true;

    if(type & FF_PATHTYPE_LINK && mode == S_IFLNK)
        return true;

    return false;
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

bool ffSuppressIO(bool suppress)
{
    static bool init = false;
    static int origOut = -1;
    static int origErr = -1;
    static int nullFile = -1;

    if(!init)
    {
        if(!suppress)
            return true;

        origOut = dup(STDOUT_FILENO);
        origErr = dup(STDERR_FILENO);
        nullFile = open("/dev/null", O_WRONLY);
        init = true;
    }

    if(nullFile == -1)
        return false;

    fflush(stdout);
    fflush(stderr);

    dup2(suppress ? nullFile : origOut, STDOUT_FILENO);
    dup2(suppress ? nullFile : origErr, STDERR_FILENO);
    return true;
}

void listFilesRecursively(FFstrbuf* folder, uint8_t indentation, const char* folderName)
{
    DIR* dir = opendir(folder->chars);
    if(dir == NULL)
        return;

    uint32_t folderLength = folder->length;

    if(folderName != NULL)
        printf("%s/\n", folderName);

    struct dirent* entry;

    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_type == DT_DIR)
        {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            ffStrbufAppendS(folder, entry->d_name);
            ffStrbufAppendC(folder, '/');
            listFilesRecursively(folder, (uint8_t) (indentation + 1), entry->d_name);
            ffStrbufSubstrBefore(folder, folderLength);
            continue;
        }

        for(uint8_t i = 0; i < indentation; i++)
            fputs("  | ", stdout);

        puts(entry->d_name);
    }

    closedir(dir);
}

void ffListFilesRecursively(const char* path)
{
    FFstrbuf folder;
    ffStrbufInitS(&folder, path);
    ffStrbufEnsureEndsWithC(&folder, '/');
    listFilesRecursively(&folder, 0, NULL);
    ffStrbufDestroy(&folder);
}
