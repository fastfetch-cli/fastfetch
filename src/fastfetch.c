#include "fastfetch.h"
#include "fastfetch_config.h"

#include <string.h>

static void printHelp()
{
    puts(
        "usage: fastfetch <options>\n"
        "\n"
        "   -h,           --help:              shows this message and exits\n"
        "   -h <command>, --help <command>:    shows help for a specific command and exits\n"
        "   -v            --version            prints the version of fastfetch and exits\n"
        "   -l <name>,    --logo <name>:       sets the shown logo. Also changes the main color accordingly\n"
        "   -c <color>,   --color <color>:     sets the color of the keys. Must be a linux console color code\n"
        "   -s <width>,   --seperator <width>: sets the distance between logo and text\n"
        "   -x <offset>,  --offsetx <offset>:  sets the x offset. Can be negative to cut the logo, but no more than logo width.\n"
        "                 --no-color:          disables all colors. This MUST be before -l/--logo or --print-logos\n"
        "                 --no-color-logo:     disables the coloring of the logo. This MUST be before -l/--logo or --print-logos\n"
        "   -r            --recache:           dont use cached values and generate new ones\n"
        "                 --show-errors:       if an error occurs, show it instead of discarding the category\n"
        "                 --list-logos:        lists the names of available logos and exits\n"
        "                 --print-logos:       prints available logos and exits\n"
    );
}

static void printCommandHelpColor()
{
    puts(
        "usage: fastfetch --color <color>\n"
        "\n"
        "<color> must be a color encoding for linux terminals. It is inserted between \"ESC[\" and \"m\".\n"
        "Infos about them can be found here: https://en.wikipedia.org/wiki/ANSI_escape_code#Colors.\n"
        "Examples:\n"
        "   \"--color 35\":    sets the color to pink\n"
        "   \"--color 4;92\":  sets the color to bright Green with underline\n"
        "   \"--color 5;104\": blinking text on a blue background\n"
        "If no color is set, the main color of the logo will be used.\n"
    );
}

static void printCommandHelp(const char* command)
{
    if(strcmp(command, "c") == 0 || strcmp(command, "-c") == 0 || strcmp(command, "color") == 0 || strcmp(command, "--color") == 0)
        printCommandHelpColor();
    else
        printf("No specific help for command %s provided\n", command);
}

static void parseArguments(int argc, char** argv, FFstate* state)
{
    bool colorText = true;

    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            if(i == argc - 1)
                printHelp();
            else
                printCommandHelp(argv[i + 1]);

            exit(0);
        }
        else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            puts(FASTFETCH_PROJECT_NAME" "FASTFETCH_PROJECT_VER);
            exit(0);
        }
        else if(strcmp(argv[i], "--list-logos") == 0)
        {
            ffListLogos();
            exit(0);
        }
        else if(strcmp(argv[i], "--print-logos") == 0)
        {
            ffPrintLogos(state->colorLogo);
            exit(0);
        }
        else if(strcmp(argv[i], "--show-errors") == 0)
        {
            state->showErrors = true;
        }
        else if(strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--logo") == 0)
        {
            if(i == argc - 1)
            {
                printf("Error: usage: %s <logo>\n", argv[i]);
                exit(41);
            }

            ffLoadLogoSet(state, argv[i + 1]);
            ++i;
        }
        else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--color") == 0)
        {
            if(i == argc - 1)
            {
                printf("Error: usage: %s <color>\n", argv[i]);
                exit(42);
            }
            size_t len = strlen(argv[i + 1]);
            if(len > 28)
            {
                printf("Error: max color string length is 28, %zu given\n", len);
                exit(43);
            }
            sprintf(state->color, "\033[%sm", argv[i + 1]);
            ++i;
        }
        else if(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--seperator") == 0)
        {
            if(i == argc -1)
            {
                printf("Error: usage: %s <width>\n", argv[i]);
                exit(44);
            }
            if(sscanf(argv[i + 1], "%hd", &state->logo_seperator) != 1)
            {
                printf("Error: couldn't parse %s to uint16_t\n", argv[i + 1]);
                exit(45);
            }
            ++i;
        }
        else if(strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--offsetx") == 0)
        {
            if(i == argc -1)
            {
                printf("Error: usage: %s <offset>\n", argv[i]);
                exit(46);
            }
            if(sscanf(argv[i + 1], "%hi", &state->offsetx) != 1)
            {
                printf("Error: couldn't parse %s to int16_t\n", argv[i + 1]);
                exit(47);
            }
            ++i;
        }
        else if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--recache") == 0)
        {
            state->recache = true;
        }
        else if(strcmp(argv[i], "--no-color") == 0)
        {
            colorText = false;
            state->colorLogo = false;
        }
        else if(strcmp(argv[i], "--no-color-logo") == 0)
        {
            state->colorLogo = false;
        }
        else
        {
            printf("Error: unknown option: %s\n", argv[i]);
            exit(40);
        }
    }

    ffLoadLogo(state); //We need to do this here, because of --no-color
    if(colorText)
        strcpy(state->color, state->logo.color);
    else
        state->color[0] = '\0';
}

int main(int argc, char** argv)
{
    FFstate state;
    ffInitState(&state);
    parseArguments(argc, argv, &state);

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
