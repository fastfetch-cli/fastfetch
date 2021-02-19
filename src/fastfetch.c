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
    ffPrintLogoAndKey(state, key);
    printf(FASTFETCH_TEXT_MODIFIER_ERROR"%s\n"FASTFETCH_TEXT_MODIFIER_RESET, message);
}

static void printHelp()
{
    puts(
        "usage: fastfetch <options>\n"
        "   -h,         --help:          shows this message and exits\n"
        "   -l <name>,  --logo <name>:   sets the shown logo. Also changes the main color accordingly\n"
        "   -c <color>, --color <color>: sets the color of the keys. Must be a linux console color code\n"
        "               --show-errors:   if an error occurs, show it instead of discarding the category\n"
        "               --list-logos:    lists the names of available logos and exits\n"
        "               --print-logos:   prints available logos and exits\n"
    );
}

int main(int argc, char** argv)
{
    FFstate state;

    ffLoadLogo(&state); //also sets color
    state.current_row = 0;
    state.passwd = getpwuid(getuid());
    uname(&state.utsname);
    sysinfo(&state.sysinfo);
    state.logo_seperator = 4;
    state.titleLength = 20; // This is overwritten by ffPrintTitle
    state.showErrors = false;

    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            printHelp();
            return 0;
        }
        else if(strcmp(argv[i], "--list-logos") == 0)
        {
            ffListLogos();
            return 0;
        }
        else if(strcmp(argv[i], "--print-logos") == 0)
        {
            ffPrintLogos();
            return 0;
        }
        else if(strcmp(argv[i], "--show-errors") == 0)
        {
            state.showErrors = true;
        }
        else if(strcmp(argv[i], "--logo") == 0 || strcmp(argv[i], "-l") == 0)
        {
            if(i == argc - 1)
            {
                printf("Error: usage: %s <logo>\n", argv[i]);
                return 40;
            }

            ffLoadLogoSet(&state, argv[i + 1]);
            ++i;
        }
        else if(strcmp(argv[i], "--color") == 0 || strcmp(argv[i], "-c") == 0)
        {
            if(i == argc - 1)
            {
                printf("Error: usage: %s <color>\n", argv[i]);
                return 41;
            }
            size_t len = strlen(argv[i + 1]);
            if(len >= 10)
            {
                printf("Error: max color string length is 10, %zu given\n", len);
                return 42;
            }
            strcpy(state.color, argv[i + 1]);
            ++i;
        }
        else
        {
            printf("Error: unknown options: %s\n", argv[i]);
            return 43;
        }
    }

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
