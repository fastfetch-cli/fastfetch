#include "packages.h"

#include <handleapi.h>
#include <fileapi.h>

static uint32_t getNumElements(const char* searchPath /* including `\*` suffix */, DWORD type)
{
    uint32_t counter = 0;
    WIN32_FIND_DATAA wfd;
    HANDLE hFind = FindFirstFileA(searchPath, &wfd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do // Managed to locate and create an handle to that folder.
        {
            if(wfd.dwFileAttributes & type)
                counter++;
        } while (FindNextFileA(hFind, &wfd));
        FindClose(hFind);
    }

    if(type == FILE_ATTRIBUTE_DIRECTORY && counter >= 2)
        counter -= 2; // accounting for . and ..

    return counter;
}

void ffDetectPackagesImpl(const FFinstance* instance, FFPackagesResult* result)
{
    FF_UNUSED(instance);

    FFstrbuf scoopPath;
    ffStrbufInitS(&scoopPath, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&scoopPath, "/scoop/apps/*");
    result->scoop = getNumElements(scoopPath.chars, FILE_ATTRIBUTE_DIRECTORY);
    if(result->scoop > 0)
        result->scoop--; // scoop
    ffStrbufDestroy(&scoopPath);
}
