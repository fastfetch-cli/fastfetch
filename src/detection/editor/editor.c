#include "editor.h"
#include "common/processing.h"
#include "util/stringUtils.h"

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
static inline char* realpath(const char* restrict file_name, char* restrict resolved_name)
{
    return _fullpath(resolved_name, file_name, _MAX_PATH);
}
#endif

const char* ffDetectEditor(FFEditorResult* result)
{
    ffStrbufSetS(&result->name, getenv("VISUAL"));
    if (result->name.length)
        result->type = "Visual";
    else
    {
        ffStrbufSetS(&result->name, getenv("EDITOR"));
        if (result->name.length)
            result->type = "Editor";
        else
            return "$VISUAL or $EDITOR not set";
    }

    #ifndef _WIN32
    if (result->name.chars[0] != '/')
    {
        if (ffProcessAppendStdOut(&result->path, (char* const[]){
            FASTFETCH_TARGET_DIR_USR "/bin/which",
            result->name.chars,
            NULL,
        }) != NULL || result->path.length == 0)
            return NULL;
    }
    #else
    if (!(result->name.length > 3 && ffCharIsEnglishAlphabet(result->name.chars[0]) && result->name.chars[1] == ':' && result->name.chars[2] == '\\'))
    {
        char buf[32];
        uint32_t len = GetSystemDirectoryA(buf, sizeof(buf));
        if (len < strlen("C:\\WINDOWS\\system32")) return NULL;
        strncpy(buf + len, "\\where.exe", sizeof(buf) - len);
        if (ffProcessAppendStdOut(&result->path, (char* const[]){
            buf,
            result->name.chars,
            NULL,
        }) != NULL || result->path.length == 0)
            return NULL;
    }
    #endif
    else
        ffStrbufSet(&result->path, &result->name);

    char buf[PATH_MAX + 1];
    if (!realpath(result->path.chars, buf))
        return NULL;

    ffStrbufSetS(&result->path, buf);

    {
        uint32_t index = ffStrbufLastIndexC(&result->path,
            #ifndef _WIN32
            '/'
            #else
            '\\'
            #endif
        );
        if (index == result->path.length)
            return NULL;
        ffStrbufSetS(&result->exe, &result->path.chars[index + 1]);
        if (!result->exe.length)
            return NULL;

        #ifdef _WIN32
        if (ffStrbufEndsWithS(&result->exe, ".exe"))
            ffStrbufSubstrBefore(&result->exe, result->exe.length - 4);
        #endif
    }

    const char* param = NULL;
    if (
        ffStrbufEqualS(&result->exe, "nano") ||
        ffStrbufEqualS(&result->exe, "vim") ||
        ffStrbufEqualS(&result->exe, "nvim") ||
        ffStrbufEqualS(&result->exe, "micro") ||
        ffStrbufEqualS(&result->exe, "emacs") ||
        ffStrbufStartsWithS(&result->exe, "emacs-") || // emacs-29.3
        ffStrbufEqualS(&result->exe, "hx") ||
        ffStrbufEqualS(&result->exe, "code") ||
        ffStrbufEqualS(&result->exe, "sublime_text")
    ) param = "--version";
    else if (
        ffStrbufEqualS(&result->exe, "kak") ||
        ffStrbufEqualS(&result->exe, "pico")
    ) param = "-version";
    else if (
        ffStrbufEqualS(&result->exe, "ne")
    ) param = "-h";
    else return NULL;

    ffProcessAppendStdOut(&result->version, (char* const[]){
        result->path.chars,
        (char*) param,
        NULL,
    });

    if (result->version.length == 0)
        return NULL;

    ffStrbufSubstrBeforeFirstC(&result->version, '\n');
    for (uint32_t iStart = 0; iStart < result->version.length; ++iStart)
    {
        char c = result->version.chars[iStart];
        if (ffCharIsDigit(c))
        {
            for (uint32_t iEnd = iStart + 1; iEnd < result->version.length; ++iEnd)
            {
                char c = result->version.chars[iEnd];
                if (isspace(c))
                {
                    ffStrbufSubstrBefore(&result->version, iEnd);
                    break;
                }
            }
            if (iStart > 0)
                ffStrbufSubstrAfter(&result->version, iStart - 1);
            break;
        }
    }

    return NULL;
}
