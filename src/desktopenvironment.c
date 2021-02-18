#include "fastfetch.h"

#include <string.h>
#include <malloc.h>

static void printKDE()
{
    char* line = NULL;
    char version[256];
    size_t len;
    
    FILE* plasma = fopen("/usr/share/xsessions/plasma.desktop", "r");
    if(plasma == NULL)
    {
        fputs("Error opening file: /usr/share/xsessions/plasma.desktop\n", stderr);
        exit(35);
    }

    while (getline(&line, &len, plasma) != -1)
    {
        if (sscanf(line, "X-KDE-PluginInfo-Version=%[^\n]", version) > 0)
            break;
    }

    fclose(plasma);
    free(line);
    printf("KDE Plasma %s", version);
}

void ffPrintDesktopEnvironment(FFstate* state)
{
    const char* currentDesktop = getenv("XDG_CURRENT_DESKTOP");
    if(currentDesktop == NULL)
        return;

    ffPrintLogoAndKey(state, "DE");

    if(strcmp(currentDesktop, "KDE") == 0)
        printKDE();
    else
        printf("%s", currentDesktop);

    const char* sessionType = getenv("XDG_SESSION_TYPE");
    if(sessionType == NULL)
        putchar('\n');
    else if(strcmp(sessionType, "wayland") == 0)
        puts(" (Wayland)");
    else if(strcmp(sessionType, "x11") == 0)
        puts(" (X11)");
    else if(strcmp(sessionType, "tty") == 0)
        puts(" TTY");
    else if(strcmp(sessionType, "mir") == 0)
        puts(" (Mir)");
    else
        printf(" (%s)\n", sessionType);
}