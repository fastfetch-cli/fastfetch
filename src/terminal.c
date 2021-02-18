#include "fastfetch.h"

#include <string.h>

void printTerminalName(const char* pid)
{
    char file[256];
    sprintf(file, "/proc/%s/stat", pid);

    FILE* stat = fopen(file, "r");
    if(stat == NULL)
    {
        printf("[Error opening %s]", file);
        return;
    }

    char name[256];
    char ppid[256];

    fscanf(stat, "%*s (%[^)])%*s%s", name, ppid);

    fclose(stat);

    if (
        strcmp(name, "bash") == 0 ||
        strcmp(name, "sh")   == 0 ||
        strcmp(name, "zsh")  == 0 ||
        strcmp(name, "ksh")  == 0 ||
        strcmp(name, "fish") == 0 ||
        strcmp(name, "sudo") == 0 ||
        strcmp(name, "su")   == 0 ||
        strcmp(name, "doas") == 0 )
    {
        printTerminalName(ppid);
    }
    else if(
        strcmp(name, "systemd") == 0 ||
        strcmp(name, "init")    == 0 ||
        strcmp(ppid, "0")       == 0 )
    {
        puts("TTY");
    }
    else
    {
        printf("%s\n", name);
    }
}

void ffPrintTerminal(FFstate* state)
{
    char ppid[256];
    sprintf(ppid, "%i", getppid());

    ffPrintLogoAndKey(state, "Terminal");
    printTerminalName(ppid);
}
