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

    char* shellName = strrchr(shellPath, '/');

    if(shellName == NULL)
        shellName = shellPath;
    else if(shellName[0] == '/')
        ++shellName;

    ffPrintAndSaveCachedValue(instance, "Shell", shellName);
}