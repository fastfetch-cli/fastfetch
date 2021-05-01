#include "fastfetch.h"

#include <string.h>
#include <ctype.h>

#define FF_SHELL_MODULE_NAME "Shell"
#define FF_SHELL_NUM_FORMAT_ARGS 4

void ffPrintShell(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_SHELL_MODULE_NAME, &instance->config.shellKey, &instance->config.shellFormat, FF_SHELL_NUM_FORMAT_ARGS))
        return;

    char* shellPath = getenv("SHELL");
    if(shellPath == NULL)
    {
        ffPrintError(instance, FF_SHELL_MODULE_NAME, 0, &instance->config.shellKey, &instance->config.shellFormat, FF_SHELL_NUM_FORMAT_ARGS, "getenv(\"SHELL\") == NULL");
        return;
    }

    char* shellName = strrchr(shellPath, '/');
    if(shellName != NULL)
        ++shellName;

    FFstrbuf version;
    ffStrbufInit(&version);

    FFstrbuf command;
    ffStrbufInit(&command);
    ffStrbufAppendS(&command, "printf %s \"$");
    ffStrbufAppendTransformS(&command, shellName != NULL ? shellName : shellPath, toupper);
    ffStrbufAppendS(&command, "_VERSION\"");

    ffProcessAppendStdOut(&version, (char* const[]){
        shellPath,
        "-c",
        command.chars,
        NULL
    });

    ffStrbufDestroy(&command);

    FFstrbuf versionPretty;
    ffStrbufInitCopy(&versionPretty, &version);
    ffStrbufSubstrBeforeFirstC(&versionPretty, '(');
    ffStrbufRemoveStrings(&versionPretty, 1, "-release");

    char null = '\0';

    if(shellName == NULL)
    {
        shellName = shellPath;
        shellPath = &null;
    }
    else
    {
        *(shellName - 1) = '\0';
    }

    FF_STRBUF_CREATE(shell);
    ffStrbufSetS(&shell, shellName);

    if(versionPretty.length > 0)
    {
        ffStrbufAppendC(&shell, ' ');
        ffStrbufAppend(&shell, &versionPretty);
    }
    else if(version.length > 0)
    {
        ffStrbufAppendC(&shell, ' ');
        ffStrbufAppend(&shell, &version);
    }

    ffPrintAndSaveToCache(instance, FF_SHELL_MODULE_NAME, &instance->config.shellKey, &shell, &instance->config.shellFormat, FF_SHELL_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRING, shellPath},
        {FF_FORMAT_ARG_TYPE_STRING, shellName},
        {FF_FORMAT_ARG_TYPE_STRBUF, &version},
        {FF_FORMAT_ARG_TYPE_STRBUF, &versionPretty}
    });

    ffStrbufDestroy(&shell);
    ffStrbufDestroy(&versionPretty);
    ffStrbufDestroy(&version);
}
