#include "io.h"
#include "fastfetch.h"
#include "util/stringUtils.h"

#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <dirent.h>
#include <errno.h>

#if __has_include(<wordexp.h>)
#include <wordexp.h>
#endif

static void createSubfolders(const char* fileName)
{
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();

    char *token = NULL;
    while((token = strchr(fileName, '/')) != NULL)
    {
        ffStrbufAppendNS(&path, (uint32_t)(token - fileName + 1), fileName);
        mkdir(path.chars, S_IRWXU | S_IRGRP | S_IROTH);
        fileName = token + 1;
    }
}

bool ffWriteFileData(const char* fileName, size_t dataSize, const void* data)
{
    int openFlagsModes = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC;
    int openFlagsRights = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    int FF_AUTO_CLOSE_FD fd = open(fileName, openFlagsModes, openFlagsRights);
    if(fd == -1)
    {
        if (errno == ENOENT)
        {
            createSubfolders(fileName);
            fd = open(fileName, openFlagsModes, openFlagsRights);
            if(fd == -1)
                return false;
        }
        else
            return false;
    }

    return write(fd, data, dataSize) > 0;
}

static inline void readWithLength(int fd, FFstrbuf* buffer, uint32_t length)
{
    ffStrbufEnsureFixedLengthFree(buffer, length);
    ssize_t bytesRead = 0;
    while(
        length > 0 && (bytesRead = read(fd, buffer->chars + buffer->length, length)) > 0
    ) {
        buffer->length += (uint32_t) bytesRead;
        length -= (uint32_t) bytesRead;
    }
}

static inline void readUntilEOF(int fd, FFstrbuf* buffer)
{
    ffStrbufEnsureFree(buffer, 31);
    uint32_t available = ffStrbufGetFree(buffer);
    ssize_t bytesRead = 0;
    while(
        (bytesRead = read(fd, buffer->chars + buffer->length, available)) > 0
    ) {
        buffer->length += (uint32_t) bytesRead;
        if((uint32_t) bytesRead == available)
            ffStrbufEnsureFree(buffer, buffer->allocated - 1); // Doubles capacity every round. -1 for the null byte.
        available = ffStrbufGetFree(buffer);
    }
}

bool ffAppendFDBuffer(int fd, FFstrbuf* buffer)
{
    struct stat fileInfo;
    if(fstat(fd, &fileInfo) != 0)
        return false;

    if (fileInfo.st_size > 0)
        readWithLength(fd, buffer, (uint32_t)fileInfo.st_size);
    else
        readUntilEOF(fd, buffer);

    buffer->chars[buffer->length] = '\0';

    return buffer->length > 0;
}

ssize_t ffReadFileData(const char* fileName, size_t dataSize, void* data)
{
    int FF_AUTO_CLOSE_FD fd = open(fileName, O_RDONLY | O_CLOEXEC);
    if(fd == -1)
        return -1;

    return ffReadFDData(fd, dataSize, data);
}

bool ffAppendFileBuffer(const char* fileName, FFstrbuf* buffer)
{
    int FF_AUTO_CLOSE_FD fd = open(fileName, O_RDONLY | O_CLOEXEC);
    if(fd == -1)
        return false;

    return ffAppendFDBuffer(fd, buffer);
}

bool ffPathExpandEnv(FF_MAYBE_UNUSED const char* in, FF_MAYBE_UNUSED FFstrbuf* out)
{
    bool result = false;

    #if __has_include(<wordexp.h>) // https://github.com/termux/termux-packages/pull/7056

    wordexp_t exp;
    if(wordexp(in, &exp, 0) != 0)
        return false;

    if (exp.we_wordc == 1)
    {
        result = true;
        ffStrbufSetS(out, exp.we_wordv[0]);
    }

    wordfree(&exp);

    #endif

    return result;
}

const char* ffGetTerminalResponse(const char* request, const char* format, ...)
{
    int fin = STDIN_FILENO, fout = STDOUT_FILENO;
    FF_AUTO_CLOSE_FD int ftty = -1;

    if (!isatty(STDIN_FILENO))
    {
        if (ftty < 0)
            ftty = open("/dev/tty", O_RDWR | O_NOCTTY | O_CLOEXEC);
        fin = ftty;
    }
    if (!isatty(STDOUT_FILENO))
    {
        if (ftty < 0)
            ftty = open("/dev/tty", O_RDWR | O_NOCTTY | O_CLOEXEC);
        fout = ftty;
    }

    struct termios oldTerm;
    if(tcgetattr(fin, &oldTerm) == -1)
        return "tcgetattr(STDIN_FILENO, &oldTerm) failed";

    struct termios newTerm = oldTerm;
    newTerm.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    if(tcsetattr(fin, TCSAFLUSH, &newTerm) == -1)
        return "tcsetattr(STDIN_FILENO, TCSAFLUSH, &newTerm)";

    ffWriteFDData(fout, strlen(request), request);

    //Give the terminal some time to respond
    if(poll(&(struct pollfd) { .fd = fin, .events = POLLIN }, 1, FF_IO_TERM_RESP_WAIT_MS) <= 0)
    {
        tcsetattr(fin, TCSANOW, &oldTerm);
        return "poll() timeout or failed";
    }

    char buffer[512];
    ssize_t bytesRead = read(fin, buffer, sizeof(buffer) - 1);

    tcsetattr(fin, TCSANOW, &oldTerm);

    if(bytesRead <= 0)
        return "read(STDIN_FILENO, buffer, sizeof(buffer) - 1) failed";

    buffer[bytesRead] = '\0';

    va_list args;
    va_start(args, format);
    vsscanf(buffer, format, args);
    va_end(args);

    return NULL;
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
        nullFile = open("/dev/null", O_WRONLY | O_CLOEXEC);
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

void listFilesRecursively(uint32_t baseLength, FFstrbuf* folder, uint8_t indentation, const char* folderName, bool pretty)
{
    FF_AUTO_CLOSE_FD int dfd = open(folder->chars, O_RDONLY);
    if (dfd < 0)
        return;

    DIR* dir = fdopendir(dfd);
    if(dir == NULL)
        return;

    uint32_t folderLength = folder->length;

    if(pretty && folderName != NULL)
    {
        for(uint8_t i = 0; i < indentation - 1; i++)
            fputs("  | ", stdout);
        printf("%s/\n", folderName);
    }

    struct dirent* entry;

    while((entry = readdir(dir)) != NULL)
    {
        bool isDir = false;
#ifdef _DIRENT_HAVE_D_TYPE
        if(entry->d_type != DT_UNKNOWN && entry->d_type != DT_LNK)
            isDir = entry->d_type == DT_DIR;
        else
#else
        {
            struct stat stbuf;
            if (fstatat(dfd, entry->d_name, &stbuf, 0) < 0)
                isDir = false;
            else
                isDir = S_ISDIR(stbuf.st_mode);
        }
#endif
        if (isDir)
        {
            if(ffStrEquals(entry->d_name, ".") || ffStrEquals(entry->d_name, ".."))
                continue;

            ffStrbufAppendS(folder, entry->d_name);
            ffStrbufAppendC(folder, '/');
            listFilesRecursively(baseLength, folder, (uint8_t) (indentation + 1), entry->d_name, pretty);
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

        puts(entry->d_name);
    }

    closedir(dir);
}

void ffListFilesRecursively(const char* path, bool pretty)
{
    FF_STRBUF_AUTO_DESTROY folder = ffStrbufCreateS(path);
    ffStrbufEnsureEndsWithC(&folder, '/');
    listFilesRecursively(folder.length, &folder, 0, NULL, pretty);
}
