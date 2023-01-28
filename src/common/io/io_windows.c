#include "io.h"

static void createSubfolders(const char* fileName)
{
    FFstrbuf path;
    ffStrbufInit(&path);

    while(*fileName != '\0')
    {
        ffStrbufAppendC(&path, *fileName);
        if(*fileName == '/')
            CreateDirectoryA(path.chars, NULL);
        ++fileName;
    }

    ffStrbufDestroy(&path);
}

bool ffWriteFileData(const char* fileName, size_t dataSize, const void* data)
{
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

    return ret;
}

bool ffAppendFDBuffer(HANDLE handle, FFstrbuf* buffer)
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

ssize_t ffReadFileData(const char* fileName, size_t dataSize, void* data)
{
    HANDLE handle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(handle == INVALID_HANDLE_VALUE)
        return false;

    DWORD readed;
    if(!ReadFile(handle, data, (DWORD)dataSize, &readed, NULL))
        return -1;

    return (ssize_t)readed;
}

bool ffAppendFileBuffer(const char* fileName, FFstrbuf* buffer)
{
    HANDLE handle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(handle == INVALID_HANDLE_VALUE)
        return false;

    bool ret = ffAppendFDBuffer(handle, buffer);

    CloseHandle(handle);

    return ret;
}

bool ffPathExists(const char* path, FFPathType type)
{
    DWORD attr = GetFileAttributesA(path);
    if(attr == INVALID_FILE_ATTRIBUTES)
        return false;

    if(type & FF_PATHTYPE_REGULAR && !(attr & FILE_ATTRIBUTE_DIRECTORY))
        return true;

    if(type & FF_PATHTYPE_DIRECTORY && (attr & FILE_ATTRIBUTE_DIRECTORY))
        return true;

    if(type & FF_PATHTYPE_LINK && (attr & FILE_ATTRIBUTE_REPARSE_POINT))
        return true;

    return false;
}

void ffSuppressIO(bool suppress)
{
    FF_UNUSED(suppress);
}
