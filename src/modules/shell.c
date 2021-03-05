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

    char* value;

    if(!instance->config.shellShowPath)
    {
        char* shellName = strrchr(shellPath, '/');
        if(shellName != NULL)
            value = ++shellName;
        else
            value = shellPath;
    }
    else
    {
        value = shellPath;
    }

    ffPrintAndSaveCachedValue(instance, "Shell", value);
}