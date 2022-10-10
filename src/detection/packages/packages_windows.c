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
        } while (FindNextFileA(hFind, &wfd) == TRUE);
        FindClose(hFind);
    }

    return counter;
}

#ifdef __MSYS__
    void ffDetectPackagesPosix(const FFinstance* instance, FFPackageCounts* counts);
#endif

void ffDetectPackages(FFinstance* instance, FFPackageCounts* counts)
{
    #ifdef __MSYS__
        //We have pacman and maybe others in MSYS, but not package managers for Windows
        if(getenv("MSYSTEM"))
            return ffDetectPackagesPosix(instance, counts);
    #endif

    FFstrbuf scoopPath;
    ffStrbufInitF(&scoopPath, "%s/scoop/apps/*", getenv("USERPROFILE"));
    counts->scoop = getNumElements(scoopPath.chars, FILE_ATTRIBUTE_DIRECTORY);
    if(counts->scoop >= 3)
        counts->scoop -= 3; // . .. scoop
    else
        counts->scoop = 0;
    ffStrbufDestroy(&scoopPath);
}
