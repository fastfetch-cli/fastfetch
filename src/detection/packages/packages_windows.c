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

static void detectScoop(const FFinstance* instance, FFPackagesResult* result)
{
    char scoopPath[MAX_PATH + 3];
    strcpy(scoopPath, instance->state.platform.homeDir.chars);
    strncat(scoopPath, "/scoop/apps/*", sizeof(scoopPath) - 1 - strlen(scoopPath));
    result->scoop = getNumElements(scoopPath, FILE_ATTRIBUTE_DIRECTORY);
    if(result->scoop > 0)
        result->scoop--; // scoop
}

static void detectChoco(const FFinstance* instance, FFPackagesResult* result)
{
    FF_UNUSED(instance);

    const char* chocoInstall = getenv("ChocolateyInstall");
    if(!chocoInstall || chocoInstall[0] == '\0')
        return;

    char chocoPath[MAX_PATH + 3];
    strcpy(chocoPath, chocoInstall);
    strncat(chocoPath, "/lib/*", sizeof(chocoPath) - 1 - strlen(chocoPath));
    result->choco = getNumElements(chocoPath, FILE_ATTRIBUTE_DIRECTORY);
    if(result->choco > 0)
        result->choco--; // choco
}

void ffDetectPackagesImpl(const FFinstance* instance, FFPackagesResult* result)
{
    detectScoop(instance, result);
    detectChoco(instance, result);
}
