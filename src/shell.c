#include "fastfetch.h"

#include <string.h>

void ffPrintShell(FFstate* state)
{
    char* shellPath = getenv("SHELL");
    char* shellName = strrchr(shellPath, '/');

    if(shellName == NULL)
        shellName = shellPath;
    else if(shellName[0] == '/')
        ++shellName;

    ffPrintLogoAndKey(state, "Shell");
    printf("%s\n", shellName);
}