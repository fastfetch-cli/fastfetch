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

static void parseArguments(int argc, char** argv, FFconfig* config)
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
            ffPrintLogos(config->colorLogo);
            exit(0);
        }
        else if(strcmp(argv[i], "--show-errors") == 0)
        {
            config->showErrors = true;
        }
        else if(strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--logo") == 0)
        {
            if(i == argc - 1)
            {
                printf("Error: usage: %s <logo>\n", argv[i]);
                exit(41);
            }

            ffLoadLogoSet(config, argv[i + 1]);
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
            sprintf(config->color, "\033[%sm", argv[i + 1]);
            ++i;
        }
        else if(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--seperator") == 0)
        {
            if(i == argc -1)
            {
                printf("Error: usage: %s <width>\n", argv[i]);
                exit(44);
            }
            if(sscanf(argv[i + 1], "%hd", &config->logo_seperator) != 1)
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
            if(sscanf(argv[i + 1], "%hi", &config->offsetx) != 1)
            {
                printf("Error: couldn't parse %s to int16_t\n", argv[i + 1]);
                exit(47);
            }
            ++i;
        }
        else if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--recache") == 0)
        {
            config->recache = true;
        }
        else if(strcmp(argv[i], "--no-color") == 0)
        {
            colorText = false;
            config->colorLogo = false;
        }
        else if(strcmp(argv[i], "--no-color-logo") == 0)
        {
            config->colorLogo = false;
        }
        else
        {
            printf("Error: unknown option: %s\n", argv[i]);
            exit(40);
        }
    }

    ffLoadLogo(config); //We need to do this here, because of --no-color
    if(colorText)
        strcpy(config->color, config->logo.color);
    else
        config->color[0] = '\0';
}

static void defaultConfig(FFconfig* config)
{
    config->logo_seperator = 4;
    config->offsetx = 0;
    config->titleLength = 20; // This is overwritten by ffPrintTitle
    config->colorLogo = true;
    config->showErrors = false;
    config->recache = false;
}

int main(int argc, char** argv)
{
    FFinstance instance;
    ffInitState(&instance.state);
    defaultConfig(&instance.config);

    parseArguments(argc, argv, &instance.config);

    //Start the printing
    ffPrintTitle(&instance);
    ffPrintSeperator(&instance);
    ffPrintOS(&instance);
    ffPrintHost(&instance);
    ffPrintKernel(&instance);
    ffPrintUptime(&instance);
    ffPrintPackages(&instance);
    ffPrintShell(&instance);
    ffPrintResolution(&instance);
    ffPrintDesktopEnvironment(&instance);
    ffPrintTheme(&instance);
    ffPrintIcons(&instance);
    ffPrintFont(&instance);
    ffPrintTerminal(&instance);
    ffPrintCPU(&instance);
    ffPrintGPU(&instance);
    ffPrintMemory(&instance);
    ffPrintDisk(&instance);
    ffPrintBattery(&instance);
    ffPrintLocale(&instance);
    ffPrintBreak(&instance);
    ffPrintColors(&instance);

    return 0;
}
