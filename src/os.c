#include "fastfetch.h"

void ffPrintOS(FFstate* state)
{
    char* line = NULL;
    char name[256];
    size_t len;
    
    FILE* os_release = fopen("/etc/os-release", "r");
    if(os_release == NULL)
        exit(2);

    while (getline(&line, &len, os_release) != -1)
    {
        if (sscanf(line, "NAME=\"%[^\"]+", name) > 0)
            break;
    }

    fclose(os_release);
    free(line);

    ffPrintLogoAndKey(state, "OS");
    printf("%s %s\n", name, state->utsname.machine);
}