#include "packages.h"
#include "common/processing.h"
#include "util/stringUtils.h"
#include "util/path.h"

#include <handleapi.h>
#include <fileapi.h>

static uint32_t getNumElements(const char* searchPath /* including `\*` suffix */, DWORD type, const char* ignore)
{
    uint32_t counter = 0;
    bool flag = ignore == NULL;
    WIN32_FIND_DATAA wfd;
    HANDLE hFind = FindFirstFileA(searchPath, &wfd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do // Managed to locate and create an handle to that folder.
        {
            if(!(wfd.dwFileAttributes & type)) continue;
            if(!flag && ffStrEqualsIgnCase(ignore, wfd.cFileName))
            {
                flag = true;
                continue;
            }
            counter++;
        } while (FindNextFileA(hFind, &wfd));
        FindClose(hFind);

        if(type == FILE_ATTRIBUTE_DIRECTORY && counter >= 2)
            counter -= 2; // accounting for . and ..
    }

    return counter;
}

static inline void wrapYyjsonFree(yyjson_doc** doc)
{
    assert(doc);
    if (*doc)
        yyjson_doc_free(*doc);
}

static void detectScoop(FFPackagesResult* result)
{
    FF_STRBUF_AUTO_DESTROY scoopPath = ffStrbufCreateA(MAX_PATH + 3);
    ffStrbufAppendS(&scoopPath, instance.state.platform.homeDir.chars);
    ffStrbufAppendS(&scoopPath, ".config/scoop/config.json");

    yyjson_val* root = NULL;

    yyjson_doc* __attribute__((__cleanup__(wrapYyjsonFree))) doc = yyjson_read_file(scoopPath.chars, 0, NULL, NULL);
    if (doc)
    {
        root = yyjson_doc_get_root(doc);
        if (!yyjson_is_obj(root)) root = NULL;
    }

    {
        ffStrbufClear(&scoopPath);
        if (root)
            ffStrbufSetJsonVal(&scoopPath, yyjson_obj_get(root, "root_path"));
        if (scoopPath.length == 0)
        {
            ffStrbufSet(&scoopPath, &instance.state.platform.homeDir);
            ffStrbufAppendS(&scoopPath, "/scoop");
        }
        ffStrbufAppendS(&scoopPath, "/apps/*");
        result->scoopUser = getNumElements(scoopPath.chars, FILE_ATTRIBUTE_DIRECTORY, "scoop");
    }

    {
        ffStrbufClear(&scoopPath);
        if (root)
            ffStrbufSetJsonVal(&scoopPath, yyjson_obj_get(root, "global_path"));
        if (scoopPath.length == 0)
        {
            ffStrbufSetS(&scoopPath, getenv("ProgramData"));
            ffStrbufAppendS(&scoopPath, "/scoop");
        }
        ffStrbufAppendS(&scoopPath, "/apps/*");
        result->scoopGlobal = getNumElements(scoopPath.chars, FILE_ATTRIBUTE_DIRECTORY, "scoop");
    }
}

static void detectChoco(FF_MAYBE_UNUSED FFPackagesResult* result)
{
    const char* chocoInstall = getenv("ChocolateyInstall");
    if(!chocoInstall || chocoInstall[0] == '\0')
        return;

    char chocoPath[MAX_PATH + 3];
    char* pend = ffStrCopy(chocoPath, chocoInstall, ARRAY_SIZE(chocoPath));
    ffStrCopy(pend, "/lib/*", ARRAY_SIZE(chocoPath) - (size_t) (pend - chocoPath));
    result->choco = getNumElements(chocoPath, FILE_ATTRIBUTE_DIRECTORY, "choco");
}

static void detectPacman(FFPackagesResult* result)
{
    const char* msystemPrefix = getenv("MSYSTEM_PREFIX");
    if(!msystemPrefix)
        return;

    // MSYS2
    char pacmanPath[MAX_PATH + 3];
    char* pend = ffStrCopy(pacmanPath, msystemPrefix, ARRAY_SIZE(pacmanPath));
    ffStrCopy(pend, "/../var/lib/pacman/local/*", ARRAY_SIZE(pacmanPath) - (size_t) (pend - pacmanPath));
    result->pacman = getNumElements(pacmanPath, FILE_ATTRIBUTE_DIRECTORY, NULL);
}

static void detectWinget(FFPackagesResult* result)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buffer, (char* []) {
        "winget.exe",
        "list",
        "--disable-interactivity",
        NULL,
    }))
        return;

    uint32_t index = ffStrbufFirstIndexS(&buffer, "--\r\n"); // Ignore garbage and table headers
    if (index == buffer.length)
        return;

    uint32_t count = 0;
    for (
        index += strlen("--\r\n");
        (index = ffStrbufNextIndexC(&buffer, index, '\n')) < buffer.length;
        ++index
    )
        ++count;

    if (buffer.chars[buffer.length - 1] != '\n') // count last line
        ++count;

    result->winget = count;
}

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options)
{
    if (!(options->disabled & FF_PACKAGES_FLAG_SCOOP_BIT)) detectScoop(result);
    if (!(options->disabled & FF_PACKAGES_FLAG_CHOCO_BIT)) detectChoco(result);
    if (!(options->disabled & FF_PACKAGES_FLAG_PACMAN_BIT)) detectPacman(result);
    if (!(options->disabled & FF_PACKAGES_FLAG_WINGET_BIT)) detectWinget(result);
}
