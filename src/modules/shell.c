#include "fastfetch.h"

#include <string.h>

#define FF_SHELL_MODULE_NAME "Shell"
#define FF_SHELL_NUM_FORMAT_ARGS 2

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

    char empty[1];
    empty[0] = '\0';

    char* shellName = strrchr(shellPath, '/');
    if(shellName == NULL)
    {
        shellName = shellPath;
        shellPath = empty;
    }
    else
    {
        *shellName = '\0';
        ++shellName;
    }

    FF_STRBUF_CREATE(shell);
    ffStrbufSetS(&shell, shellName);

    ffPrintAndSaveToCache(instance, FF_SHELL_MODULE_NAME, &instance->config.shellKey, &shell, &instance->config.shellFormat, FF_SHELL_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRING, shellPath},
        {FF_FORMAT_ARG_TYPE_STRING, shellName}
    });

    ffStrbufDestroy(&shell);
}
