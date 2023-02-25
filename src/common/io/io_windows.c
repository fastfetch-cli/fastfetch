#include "io.h"

static void createSubfolders(const char* fileName)
{
    FF_STRBUF_AUTO_DESTROY path;
    ffStrbufInit(&path);

    while(*fileName != '\0')
    {
        ffStrbufAppendC(&path, *fileName);
        if(*fileName == '/')
            CreateDirectoryA(path.chars, NULL);
        ++fileName;
    }
}

bool ffWriteFileData(const char* fileName, size_t dataSize, const void* data)
{
    HANDLE FF_AUTO_CLOSE_FD handle = CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(handle == INVALID_HANDLE_VALUE)
    {
        createSubfolders(fileName);
        handle = CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(handle == INVALID_HANDLE_VALUE)
            return false;
    }

    DWORD written;
    return !!WriteFile(handle, data, (DWORD)dataSize, &written, NULL);
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

bool ffSuppressIO(bool suppress)
{
    (void) suppress; //Not implemented.
    return false;
}

void listFilesRecursively(FFstrbuf* folder, uint8_t indentation, const char* folderName)
{
    uint32_t folderLength = folder->length;

    if(folderName != NULL)
        printf("%s/\n", folderName);

    ffStrbufAppendC(folder, '*');
    WIN32_FIND_DATAA entry;
    HANDLE hFind = FindFirstFileA(folder->chars, &entry);
    if(hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if(strcmp(entry.cFileName, ".") == 0 || strcmp(entry.cFileName, "..") == 0)
                continue;

            ffStrbufSubstrBefore(folder, folderLength);
            ffStrbufAppendS(folder, entry.cFileName);
            ffStrbufAppendC(folder, '/');
            listFilesRecursively(folder, (uint8_t) (indentation + 1), entry.cFileName);
            ffStrbufSubstrBefore(folder, folderLength);
            continue;
        }

        for(uint8_t i = 0; i < indentation; i++)
            fputs("  | ", stdout);

        puts(entry.cFileName);
    } while (FindNextFileA(hFind, &entry));
    FindClose(hFind);
}

void ffListFilesRecursively(const char* path)
{
    FFstrbuf folder;
    ffStrbufInitS(&folder, path);
    ffStrbufEnsureEndsWithC(&folder, '/');
    listFilesRecursively(&folder, 0, NULL);
    ffStrbufDestroy(&folder);
}
