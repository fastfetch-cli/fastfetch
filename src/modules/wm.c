#include "fastfetch.h"

#include <dirent.h>

static void parseProcessName(const char* name, char* buffer)
{
    if(strcasecmp(name, "kwin_wayland") == 0)
        strcpy(buffer, "KWin (Wayland)");
    else if(strcasecmp(name, "kwin_x11") == 0)
        strcpy(buffer, "KWin (X11)");        
}

void ffPrintWM(FFinstance* instance)
{
    DIR* proc = opendir("/proc/");
    if(proc == NULL)
    {
        ffPrintError(instance, "WM", "opendir(\"/proc/\") == NULL");
        return;
    }

    struct dirent* dirent;

    char name[32];
    name[0] = '\0';

    while((dirent = readdir(proc)) != NULL)
    {
        if(dirent->d_type != DT_DIR)
            continue;

        char path[20];
        sprintf(path, "/proc/%.8s/comm", dirent->d_name);

        char comm[17]; //MAX_COMM_LENGTH is 17 + null terminator
        ffGetFileContent(path, comm, sizeof(comm));

        if(comm[0] == '\0')
            continue;

        parseProcessName(comm, name);

        if(name[0] != '\0')
            break;
    }

    closedir(proc);

    if(name[0] == '\0')
    {
        ffPrintError(instance, "WM", "No process name matches the name of known display managers");
    }
    else
    {
        ffPrintLogoAndKey(instance, "WM");
        puts(name);
    }
}