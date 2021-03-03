#include "fastfetch.h"

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
        strcasecmp(name, "bash")   == 0 ||
        strcasecmp(name, "sh")     == 0 ||
        strcasecmp(name, "zsh")    == 0 ||
        strcasecmp(name, "ksh")    == 0 ||
        strcasecmp(name, "fish")   == 0 ||
        strcasecmp(name, "sudo")   == 0 ||
        strcasecmp(name, "su")     == 0 ||
        strcasecmp(name, "doas")   == 0 ||
        strcasecmp(name, "strace") == 0 )
    {
        printTerminalName(instance, ppid);
        return;
    }

    ffPrintLogoAndKey(instance, "Terminal");
    
    if(
        strcasecmp(name, "systemd") == 0 ||
        strcasecmp(name, "init")    == 0 ||
        strcasecmp(ppid, "0")       == 0 )
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
