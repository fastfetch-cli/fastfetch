#include "path.h"

#include "common/io/io.h"
#include "util/stringUtils.h"

const char* ffFindExecutableInPath(const char* name, FFstrbuf* result)
{
    char* path = getenv("PATH");
    if(!path)
        return "$PATH not set";

    #ifdef _WIN32
    const bool appendExe = !ffStrEndsWithIgnCase(name, ".exe");
    #endif

    for (char* token = path; *token; path = token + 1)
    {
        token = strchr(path,
            #ifdef _WIN32
                ';'
            #else
                ':'
            #endif
        );
        if (!token) token = path + strlen(path);

        ffStrbufSetNS(result, (uint32_t)(token - path), path);
        ffStrbufEnsureEndsWithC(result,
            #ifdef _WIN32
                '\\'
            #else
                '/'
            #endif
        );
        ffStrbufAppendS(result, name);
        #ifdef _WIN32
        if (appendExe) ffStrbufAppendS(result, ".exe");
        if (!ffPathExists(result->chars, FF_PATHTYPE_FILE))
            continue;
        #else
        struct stat st;
        if (stat(result->chars, &st) < 0 || !(st.st_mode & S_IXUSR))
            continue;
        #endif

        return NULL;
    }
    ffStrbufClear(result);
    return "Executable not found";
}

bool ffIsAbsolutePath(const char* path)
{
    #ifdef _WIN32
    return ffCharIsEnglishAlphabet(path[0]) && path[1] == ':' && (path[2] == '\\' || path[2] == '/');
    #else
    return path[0] == '/';
    #endif
}
