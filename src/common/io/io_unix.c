#include "io.h"
#include "fastfetch.h"
#include "util/stringUtils.h"

#include <fcntl.h>
#include <termios.h>
#include <dirent.h>
#include <errno.h>
#ifndef __APPLE__
#include <poll.h>
#else
#include <sys/select.h>
#endif

#if FF_HAVE_WORDEXP
    #include <wordexp.h>
#elif FF_HAVE_GLOB
    #warning "<wordexp.h> is not available, use <glob.h> instead"
    #include <glob.h>
#else
    #warning "Neither <wordexp.h> nor <glob.h> is available"
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
    mode_t openFlagsRights = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

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

ssize_t ffReadFileDataRelative(int dfd, const char* fileName, size_t dataSize, void* data)
{
    int FF_AUTO_CLOSE_FD fd = openat(dfd, fileName, O_RDONLY | O_CLOEXEC);
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

bool ffAppendFileBufferRelative(int dfd, const char* fileName, FFstrbuf* buffer)
{
    int FF_AUTO_CLOSE_FD fd = openat(dfd, fileName, O_RDONLY | O_CLOEXEC);
    if(fd == -1)
        return false;

    return ffAppendFDBuffer(fd, buffer);
}

bool ffPathExpandEnv(FF_MAYBE_UNUSED const char* in, FF_MAYBE_UNUSED FFstrbuf* out)
{
    bool result = false;

    #if FF_HAVE_WORDEXP // https://github.com/termux/termux-packages/pull/7056

    wordexp_t exp;
    if (wordexp(in, &exp, 0) != 0)
        return false;

    if (exp.we_wordc == 1)
    {
        result = true;
        ffStrbufSetS(out, exp.we_wordv[0]);
    }

    wordfree(&exp);

    #elif FF_HAVE_GLOB

    glob_t gb;
    if (glob(in, GLOB_NOSORT | GLOB_TILDE, NULL, &gb) != 0)
        return false;

    if (gb.gl_matchc == 1)
    {
        result = true;
        ffStrbufSetS(out, gb.gl_pathv[0]);
    }

    globfree(&gb);

    #endif

    return result;
}

static int ftty = -1;
static struct termios oldTerm;
void restoreTerm(void)
{
    tcsetattr(ftty, TCSAFLUSH, &oldTerm);
}

const char* ffGetTerminalResponse(const char* request, int nParams, const char* format, ...)
{
    if (ftty < 0)
    {
        ftty = open("/dev/tty", O_RDWR | O_NOCTTY | O_CLOEXEC);
        if (ftty < 0)
            return "open(\"/dev/tty\", O_RDWR | O_NOCTTY | O_CLOEXEC) failed";

        if(tcgetattr(ftty, &oldTerm) == -1)
            return "tcgetattr(STDIN_FILENO, &oldTerm) failed";

        struct termios newTerm = oldTerm;
        newTerm.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
        if(tcsetattr(ftty, TCSAFLUSH, &newTerm) == -1)
            return "tcsetattr(STDIN_FILENO, TCSAFLUSH, &newTerm)";
        atexit(restoreTerm);
    }

    ffWriteFDData(ftty, strlen(request), request);

    //Give the terminal some time to respond
    #ifndef __APPLE__
    if(poll(&(struct pollfd) { .fd = ftty, .events = POLLIN }, 1, FF_IO_TERM_RESP_WAIT_MS) <= 0)
        return "poll(/dev/tty) timeout or failed";
    #else
    {
        // On macOS, poll(/dev/tty) always returns immediately
        // See also https://nathancraddock.com/blog/macos-dev-tty-polling/
        fd_set rd;
        FD_ZERO(&rd);
        FD_SET(ftty, &rd);
        if(select(ftty + 1, &rd, NULL, NULL, &(struct timeval) { .tv_sec = FF_IO_TERM_RESP_WAIT_MS / 1000, .tv_usec = (FF_IO_TERM_RESP_WAIT_MS % 1000) * 1000 }) <= 0)
            return "select(/dev/tty) timeout or failed";
    }
    #endif

    char buffer[1024];
    size_t bytesRead = 0;

    va_list args;
    va_start(args, format);

    while (true)
    {
        ssize_t nRead = read(ftty, buffer + bytesRead, sizeof(buffer) - bytesRead - 1);

        if (nRead <= 0)
        {
            va_end(args);
            return "read(STDIN_FILENO, buffer, sizeof(buffer) - 1) failed";
        }

        bytesRead += (size_t) nRead;
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
        if(entry->d_name[0] == '.') // skip hidden files
            continue;

        bool isDir = false;
#ifndef __sun
        if(entry->d_type != DT_UNKNOWN && entry->d_type != DT_LNK)
            isDir = entry->d_type == DT_DIR;
        else
#endif
        {
            struct stat stbuf;
            if (fstatat(dfd, entry->d_name, &stbuf, 0) < 0)
                isDir = false;
            else
                isDir = S_ISDIR(stbuf.st_mode);
        }
        if (isDir)
        {
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
