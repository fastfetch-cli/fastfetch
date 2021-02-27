#include "fastfetch.h"

#include <string.h>

void printTerminalName(FFinstance* instance, const char* pid)
{
    char file[256];
    sprintf(file, "/proc/%s/stat", pid);

    FILE* stat = fopen(file, "r");
    if(stat == NULL)
    {
        char error[300];
        sprintf(error, "fopen(\"%s\", \"r\") == NULL", file);
        ffPrintError(instance, "Terminal", error);
        return;
    }

    char name[256];
    char ppid[256];
    if(fscanf(stat, "%*s (%[^)])%*s%s", name, ppid) != 2)
    {
        ffPrintError(instance, "Terminal", "fscanf(stat, \"%*s (%[^)])%*s%s\", name, ppid) != 2");
        return;
    }

    fclose(stat);

    if (
        strcmp(name, "bash")   == 0 ||
        strcmp(name, "sh")     == 0 ||
        strcmp(name, "zsh")    == 0 ||
        strcmp(name, "ksh")    == 0 ||
        strcmp(name, "fish")   == 0 ||
        strcmp(name, "sudo")   == 0 ||
        strcmp(name, "su")     == 0 ||
        strcmp(name, "doas")   == 0 ||
        strcmp(name, "strace") == 0 )
    {
        printTerminalName(instance, ppid);
        return;
    }

    ffPrintLogoAndKey(instance, "Terminal");
    
    if(
        strcmp(name, "systemd") == 0 ||
        strcmp(name, "init")    == 0 ||
        strcmp(ppid, "0")       == 0 )
    {
        puts("TTY");
    }
    else
    {
        puts(name);
    }
}

void ffPrintTerminal(FFinstance* instance)
{
    char ppid[256];
    sprintf(ppid, "%i", getppid());
    printTerminalName(instance, ppid);
}
