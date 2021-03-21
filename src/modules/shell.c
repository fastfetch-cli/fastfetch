#include "fastfetch.h"

void ffPrintShell(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, "Shell"))
        return;

    char* shellPath = getenv("SHELL");
    if(shellPath == NULL)
    {
        ffPrintError(instance, "Shell", "getenv(\"SHELL\") == NULL");
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

    ffPrintLogoAndKey(instance, "Shell");

    FF_STRBUF_CREATE(shell);

    if(!ffStrbufIsEmpty(&instance->config.shellFormat))
    {
        ffParseFormatString(&shell, &instance->config.shellFormat, 2,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, shellPath},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, shellName}
        );
    }
    else
    {
        ffStrbufSetS(&shell, shellName);
    }

    ffPrintAndSaveCachedValue(instance, "Shell", shell.chars);
    ffStrbufDestroy(&shell);
}