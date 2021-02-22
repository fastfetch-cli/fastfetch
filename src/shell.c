#include "fastfetch.h"

#include <string.h>

void ffPrintShell(FFstate* state)
{
    if(ffPrintCachedValue(state, "Shell"))
        return;

    char* shellPath = getenv("SHELL");
    if(shellPath == NULL)
    {
        ffPrintError(state, "Shell", "getenv(\"SHELL\") == NULL");
        return;
    }

    char* shellName = strrchr(shellPath, '/');

    if(shellName == NULL)
        shellName = shellPath;
    else if(shellName[0] == '/')
        ++shellName;

    ffSaveCachedValue(state, "Shell", shellName);

    ffPrintLogoAndKey(state, "Shell");
    puts(shellName);
}