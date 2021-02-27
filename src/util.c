#include "fastfetch.h"

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void ffInitState(FFstate* state)
{
    state->current_row = 0;
    state->passwd = getpwuid(getuid());
    uname(&state->utsname);
    sysinfo(&state->sysinfo);
    state->logo_seperator = 4;
    state->offsetx = 0;
    state->titleLength = 20; // This is overwritten by ffPrintTitle
    state->colorLogo = true;
    state->showErrors = false;
    state->recache = false;
}

void ffPrintKey(FFstate* state, const char* key)
{
    printf(FASTFETCH_TEXT_MODIFIER_BOLT"%s%s"FASTFETCH_TEXT_MODIFIER_RESET": ", state->color, key);
}

void ffPrintLogoAndKey(FFstate* state, const char* key)
{
    ffPrintLogoLine(state);
    ffPrintKey(state, key);
}

void ffParsePropFile(const char* fileName, const char* regex, char* buffer)
{
    buffer[0] = '\0'; //If an error occures, this is the indicator

    char* line = NULL;
    size_t len;

    FILE* file = fopen(fileName, "r");
    if(file == NULL)
        return; // handle errors in higher functions

    while (getline(&line, &len, file) != -1)
    {
        if (sscanf(line, regex, buffer) > 0)
            break;
    }

    fclose(file);
    free(line);
}

void ffParsePropFileHome(FFstate* state, const char* relativeFile, const char* regex, char* buffer)
{
    char absolutePath[512];
    strcpy(absolutePath, state->passwd->pw_dir);
    strcat(absolutePath, "/");
    strcat(absolutePath, relativeFile);

    ffParsePropFile(absolutePath, regex, buffer);
}

static inline bool strSet(const char* str)
{
    return str != NULL && str[0] != '\0';
}

void ffPrintGtkPretty(const char* gtk2, const char* gtk3, const char* gtk4)
{
    if(strSet(gtk2) && strSet(gtk3) && strSet(gtk4))
    {
        if((strcmp(gtk2, gtk3) == 0) && (strcmp(gtk2, gtk4) == 0))
            printf("%s [GTK2/3/4]", gtk2);
        else if(strcmp(gtk2, gtk3) == 0)
            printf("%s [GTK2/3], %s [GTK4]", gtk2, gtk4);
        else if(strcmp(gtk3, gtk4) == 0)
            printf("%s [GTK2], %s [GTK3/4]", gtk2, gtk3);
        else
            printf("%s [GTK2], %s [GTK3], %s [GTK4]", gtk2, gtk3, gtk4);
    }
    else if(strSet(gtk2) && strSet(gtk3))
    {
        if(strcmp(gtk2, gtk3) == 0)
            printf("%s [GTK2/3]", gtk2);
        else
            printf("%s [GTK2], %s [GTK3]", gtk2, gtk3);
    }
    else if(strSet(gtk3) && strSet(gtk4))
    {
        if(strcmp(gtk3, gtk4) == 0)
            printf("%s [GTK3/4]", gtk3);
        else
            printf("%s [GTK3], %s [GTK4]", gtk3, gtk4);
    }
    else if(strSet(gtk2))
    {
        printf("%s [GTK2]", gtk2);
    }
    else if(strSet(gtk3))
    {
        printf("%s [GTK3]", gtk3);
    }
    else if(strSet(gtk4))
    {
        printf("%s [GTK4]", gtk4);
    }
}

void ffPrintError(FFstate* state, const char* key, const char* message)
{
    if(!state->showErrors)
        return;

    ffPrintLogoAndKey(state, key);
    printf(FASTFETCH_TEXT_MODIFIER_ERROR"%s\n"FASTFETCH_TEXT_MODIFIER_RESET, message);
}

static void getCacheFileName(FFstate* state, const char* key, char* buffer)
{
    const char* xdgCache = getenv("XDG_CACHE_HOME");
    if(xdgCache == NULL)
    {
        strcpy(buffer, state->passwd->pw_dir);
        strcat(buffer, "/.cache");
    }
    else
    {
        strcpy(buffer, xdgCache);
    }

    mkdir(buffer, S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH); //I hope everybody has a cache folder but whow knews
    
    strcat(buffer, "/fastfetch/");
    mkdir(buffer, S_IRWXU | S_IRGRP | S_IROTH);
    
    strcat(buffer, key);
}

bool ffPrintCachedValue(FFstate* state, const char* key)
{
    if(state->recache)
        return false;

    char fileName[256];
    getCacheFileName(state, key, fileName);

    int fd = open(fileName, O_RDONLY);
    if(fd == -1)
        return false;

    char value[1024];

    ssize_t readed = read(fd, value, sizeof(value) - 1);
    if(readed < 1)
        return false;

    close(fd);

    value[readed] = '\0'; //Somehow read doesn't alway do this

    ffPrintLogoAndKey(state, key);
    puts(value);

    return true;
}

void ffPrintAndSaveCachedValue(FFstate* state, const char* key, const char* value)
{
    ffPrintLogoAndKey(state, key);
    puts(value);

    char fileName[256];
    getCacheFileName(state, key, fileName);

    int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd == -1)
        return;

    size_t len = strlen(value);

    bool failed = write(fd, value, len) != len;

    close(fd);

    if(failed)
        unlink(fileName);
}
