#include "packages.h"
#include "util/stringUtils.h"

#include <handleapi.h>
#include <fileapi.h>

static uint32_t getNumElements(const char* searchPath /* including `\*` suffix */, DWORD type, const char* ignore)
{
    uint32_t counter = 0;
    WIN32_FIND_DATAA wfd;
    HANDLE hFind = FindFirstFileA(searchPath, &wfd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do // Managed to locate and create an handle to that folder.
        {
            if(!(wfd.dwFileAttributes & type)) continue;
            if(ignore != NULL && strcasecmp(ignore, wfd.cFileName) == 0) continue;
            counter++;
        } while (FindNextFileA(hFind, &wfd));
        FindClose(hFind);

        if(type == FILE_ATTRIBUTE_DIRECTORY && counter >= 2)
            counter -= 2; // accounting for . and ..
    }

    return counter;
}

static void detectScoop(const FFinstance* instance, FFPackagesResult* result)
{
    FF_STRBUF_AUTO_DESTROY scoopPath;
    ffStrbufInitA(&scoopPath, MAX_PATH + 3);

    const char* scoopEnv = getenv("SCOOP");
    if(ffStrSet(scoopEnv))
    {
        ffStrbufAppendS(&scoopPath, scoopEnv);
        ffStrbufAppendS(&scoopPath, "/apps/*");
    }
    else
    {
        ffStrbufAppendS(&scoopPath, instance->state.platform.homeDir.chars);
        ffStrbufAppendS(&scoopPath, "/scoop/apps/*");
    }
    result->scoop = getNumElements(scoopPath.chars, FILE_ATTRIBUTE_DIRECTORY, "scoop");
}

static void detectChoco(FF_MAYBE_UNUSED const FFinstance* instance, FFPackagesResult* result)
{
    const char* chocoInstall = getenv("ChocolateyInstall");
    if(!chocoInstall || chocoInstall[0] == '\0')
        return;

    char chocoPath[MAX_PATH + 3];
    strcpy(chocoPath, chocoInstall);
    strncat(chocoPath, "/lib/*", sizeof(chocoPath) - 1 - strlen(chocoPath));
    result->choco = getNumElements(chocoPath, FILE_ATTRIBUTE_DIRECTORY, "choco");
}

static void detectPacman(FF_MAYBE_UNUSED const FFinstance* instance, FFPackagesResult* result)
{
    const char* msystemPrefix = getenv("MSYSTEM_PREFIX");
    if(!msystemPrefix)
        return;

    // MSYS2
    char pacmanPath[MAX_PATH + 3];
    strcpy(pacmanPath, msystemPrefix);
    strncat(pacmanPath, "/../var/lib/pacman/local/*", sizeof(pacmanPath) - 1 - strlen(pacmanPath));
    result->pacman = getNumElements(pacmanPath, FILE_ATTRIBUTE_DIRECTORY, NULL);
}

void ffDetectPackagesImpl(const FFinstance* instance, FFPackagesResult* result)
{
    detectScoop(instance, result);
    detectChoco(instance, result);
    detectPacman(instance, result);
}
