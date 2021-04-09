#include "fastfetch.h"

#include <string.h>

void ffPrintShell(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, &instance->config.shellKey, "Shell"))
        return;

    char* shellPath = getenv("SHELL");
    if(shellPath == NULL)
    {
        ffPrintError(instance, &instance->config.shellKey, "Shell", "getenv(\"SHELL\") == NULL");
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

    if(instance->config.shellFormat.length == 0)
    {
        ffStrbufSetS(&shell, shellName);
    }
    else
    {
        ffParseFormatString(&shell, &instance->config.shellFormat, 2,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, shellPath},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, shellName}
        );
    }

    ffPrintAndSaveCachedValue(instance, &instance->config.shellKey, "Shell", &shell);
    ffStrbufDestroy(&shell);
}
