#include "fastfetch.h"

#include <string.h>

void ffPrintKey(FFstate* state, const char* key)
{
    printf("%s"FASTFETCH_TEXT_MODIFIER_BOLT"%s"FASTFETCH_TEXT_MODIFIER_RESET": ", state->color, key);
}

void ffPrintLogoAndKey(FFstate* state, const char* key)
{
    ffPrintLogoLine(state);
    ffPrintKey(state, key);
}

uint32_t ffTruncateLastNewline(char* buffer, uint32_t read)
{
    if(read == 0)
        return read;

    if(buffer[read - 1] == '\n')
    {
        buffer[read - 1] = '\0';
        --read;
    }

    return read;
}

uint32_t ffReadFile(const char* fileName, char* buffer, uint32_t bufferSize)
{
    FILE* file = fopen(fileName, "r");
    if(file == NULL)
    {
        fprintf(stderr, "Failed to open file: %s\n", fileName);
        exit(3);
    }

    uint32_t read = fread(buffer, sizeof(char), bufferSize, file);
    read = ffTruncateLastNewline(buffer, read);

    fclose(file);

    return read;
}

void ffParsePropFile(const char* fileName, const char* regex, char* buffer)
{
    char* line = NULL;
    size_t len;

    FILE* file = fopen(fileName, "r");
    if(file == NULL)
    {
        buffer[0] = '\0';
        return;
    }

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

int main(int argc, char** argv)
{
    FFstate state;

    //init
    ffLoadLogo(&state); //Use ffLoadLogoSet to specify an image. Note that this also overwrites color
    state.current_row = 0;
    state.passwd = getpwuid(getuid());
    uname(&state.utsname);
    sysinfo(&state.sysinfo);


    //Configuration
    state.logo_seperator = 4;
    state.titleLength = 20; // This is overwritten by ffPrintTitle

    //Start the printing
    ffPrintTitle(&state);
    ffPrintSeperator(&state);
    ffPrintOS(&state);
    ffPrintHost(&state);
    ffPrintKernel(&state);
    ffPrintUptime(&state);
    ffPrintPackages(&state);
    ffPrintShell(&state);
    ffPrintResolution(&state);
    ffPrintDesktopEnvironment(&state);
    ffPrintTheme(&state);
    ffPrintIcons(&state);
    ffPrintFont(&state);
    ffPrintTerminal(&state);
    ffPrintCPU(&state);
    ffPrintGPU(&state);
    ffPrintMemory(&state);
    ffPrintDisk(&state);
    ffPrintBattery(&state);
    ffPrintLocale(&state);
    ffPrintBreak(&state);
    ffPrintColors(&state);

    return 0;
}
