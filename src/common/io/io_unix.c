#include "io.h"
#include "util/stringUtils.h"
#include "util/unused.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <poll.h>
#include <dirent.h>

#if __has_include(<wordexp.h>)
#include <wordexp.h>
#endif

static void createSubfolders(const char* fileName)
{
    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();

    while(*fileName != '\0')
    {
        ffStrbufAppendC(&path, *fileName);
        if(*fileName == '/')
            mkdir(path.chars, S_IRWXU | S_IRGRP | S_IROTH);
        ++fileName;
    }
}

bool ffWriteFileData(const char* fileName, size_t dataSize, const void* data)
{
    int openFlagsModes = O_WRONLY | O_CREAT | O_TRUNC;
    int openFlagsRights = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    int FF_AUTO_CLOSE_FD fd = open(fileName, openFlagsModes, openFlagsRights);
    if(fd == -1)
    {
        createSubfolders(fileName);
        fd = open(fileName, openFlagsModes, openFlagsRights);
        if(fd == -1)
            return false;
    }

    return write(fd, data, dataSize) > 0;
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
        (readed = read(fd, buffer->chars + buffer->length, free)) > 0
    ) {
        buffer->length += (uint32_t) readed;
        if((uint32_t) readed == free)
            ffStrbufEnsureFree(buffer, buffer->allocated - 1); // Doubles capacity every round. -1 for the null byte.
        free = ffStrbufGetFree(buffer);
    }

    buffer->chars[buffer->length] = '\0';

    ffStrbufTrimRight(buffer, '\n');
    ffStrbufTrimRight(buffer, ' ');

    return buffer->length > 0;
}

ssize_t ffReadFileData(const char* fileName, size_t dataSize, void* data)
{
    int FF_AUTO_CLOSE_FD fd = open(fileName, O_RDONLY);
    if(fd == -1)
        return -1;

    return ffReadFDData(fd, dataSize, data);
}

bool ffAppendFileBuffer(const char* fileName, FFstrbuf* buffer)
{
    int FF_AUTO_CLOSE_FD fd = open(fileName, O_RDONLY);
    if(fd == -1)
        return false;

    return ffAppendFDBuffer(fd, buffer);
}

bool ffPathExists(const char* path, FFPathType type)
{
    struct stat fileStat;
    if(stat(path, &fileStat) != 0)
        return false;

    unsigned int mode = fileStat.st_mode & S_IFMT;

    if(type & FF_PATHTYPE_REGULAR && mode == S_IFREG)
        return true;

    if(type & FF_PATHTYPE_DIRECTORY && mode == S_IFDIR)
        return true;

    if(type & FF_PATHTYPE_LINK && mode == S_IFLNK)
        return true;

    return false;
}

bool ffPathExpandEnv(FF_MAYBE_UNUSED const char* in, FF_MAYBE_UNUSED FFstrbuf* out)
{
    bool result = false;

    #if __has_include(<wordexp.h>) // https://github.com/termux/termux-packages/pull/7056

    wordexp_t exp;
    if(wordexp(in, &exp, WRDE_NOCMD) != 0)
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
    struct termios oldTerm, newTerm;
    if(tcgetattr(STDIN_FILENO, &oldTerm) == -1)
        return "tcgetattr(STDIN_FILENO, &oldTerm) failed";

    newTerm = oldTerm;
    newTerm.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    if(tcsetattr(STDIN_FILENO, TCSANOW, &newTerm) == -1)
        return "tcsetattr(STDIN_FILENO, TCSANOW, &newTerm)";

    fputs(request, stdout);
    fflush(stdout);

    //Give the terminal 35ms to respond
    if(poll(&(struct pollfd) { .fd = STDIN_FILENO, .events = POLLIN }, 1, FF_IO_TERM_RESP_WAIT_MS) <= 0)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldTerm);
        return "poll() timeout or failed";
    }

    char buffer[512];
    ssize_t readed = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldTerm);

    if(readed <= 0)
        return "read(STDIN_FILENO, buffer, sizeof(buffer) - 1) failed";

    buffer[readed] = '\0';

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

void listFilesRecursively(uint32_t baseLength, FFstrbuf* folder, uint8_t indentation, const char* folderName, bool pretty)
{
    DIR* dir = opendir(folder->chars);
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
        if(entry->d_type == DT_DIR)
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
