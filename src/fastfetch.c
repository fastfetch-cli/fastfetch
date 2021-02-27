#include "fastfetch.h"
#include "fastfetch_config.h"

#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

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
    if(strcasecmp(command, "c") == 0 || strcasecmp(command, "-c") == 0 || strcasecmp(command, "color") == 0 || strcasecmp(command, "--color") == 0)
        printCommandHelpColor();
    else
        printf("No specific help for command %s provided\n", command);
}

static void parseArguments(int argc, char** argv, FFconfig* config)
{
    bool colorText = true;
    int logoIndex = -1;

    for(int i = 1; i < argc; i++)
    {
        if(strcasecmp(argv[i], "-h") == 0 || strcasecmp(argv[i], "--help") == 0)
        {
            if(i == argc - 1)
                printHelp();
            else
                printCommandHelp(argv[i + 1]);

            exit(0);
        }
        else if(strcasecmp(argv[i], "-v") == 0 || strcasecmp(argv[i], "--version") == 0)
        {
            puts(FASTFETCH_PROJECT_NAME" "FASTFETCH_PROJECT_VER);
            exit(0);
        }
        else if(strcasecmp(argv[i], "--list-logos") == 0)
        {
            ffListLogos();
            exit(0);
        }
        else if(strcasecmp(argv[i], "--print-logos") == 0)
        {
            ffPrintLogos(config->colorLogo);
            exit(0);
        }
        else if(strcasecmp(argv[i], "--show-errors") == 0)
        {
            config->showErrors = true;
        }
        else if(strcasecmp(argv[i], "-l") == 0 || strcasecmp(argv[i], "--logo") == 0)
        {
            if(i == argc - 1)
            {
                printf("Error: usage: %s <logo>\n", argv[i]);
                exit(41);
            }

            logoIndex = i + 1;
            ++i;
        }
        else if(strcasecmp(argv[i], "-c") == 0 || strcasecmp(argv[i], "--color") == 0)
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
        else if(strcasecmp(argv[i], "-s") == 0 || strcasecmp(argv[i], "--seperator") == 0)
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
        else if(strcasecmp(argv[i], "-x") == 0 || strcasecmp(argv[i], "--offsetx") == 0)
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
        else if(strcasecmp(argv[i], "-r") == 0 || strcasecmp(argv[i], "--recache") == 0)
        {
            config->recache = true;
        }
        else if(strcasecmp(argv[i], "--no-color") == 0)
        {
            colorText = false;
            config->colorLogo = false;
        }
        else if(strcasecmp(argv[i], "--no-color-logo") == 0)
        {
            config->colorLogo = false;
        }
        else
        {
            printf("Error: unknown option: %s\n", argv[i]);
            exit(40);
        }
    }

    //Load logo after parsing because of --no-color and --no-color-logo
    if(logoIndex >= 0)
        ffLoadLogoSet(config, argv[logoIndex]);
    else
        ffLoadLogo(config);

    //We need to do this after loading the logo
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

static void trimTrailingWhitespaces(char* buffer)
{
    uint32_t end = 0;

    for(uint32_t i = 0; buffer[i] != '\0'; i++)
    {
        if(buffer[i] != ' ')
            end = i;
    }

    if(buffer[end + 1] == ' ')
        buffer[end + 1] = '\0';
}

static FILE* createStructureFile(const char* filename)
{
    FILE* f = fopen(filename, "wr");

    fputs(
        "Title\n"
        "Seperator\n"
        "OS\n"
        "Host\n"
        "Kernel\n"
        "Uptime\n"
        "Packages\n"
        "Shell\n"
        "Resolution\n"
        "DesktopEnvironment\n"
        "Theme\n"
        "Icons\n"
        "Font\n"
        "Terminal\n"
        "CPU\n"
        "GPU\n"
        "Memory\n"
        "Disk\n"
        "Battery\n"
        "Locale\n"
        "Break\n"
        "Colors\n"
    ,f);

    fseek(f, 0, SEEK_SET);

    return f;
}

static bool customValue(FFinstance* instance, const char* key, const char* valueDir)
{
    char fileName[256];
    strcpy(fileName, valueDir);
    strcat(fileName, key);

    int fd = open(fileName, O_RDONLY);
    if(fd == 0) //The file doesn't exist
        return false;

    char value[1024];
    
    ssize_t readed = read(fd, value, sizeof(value) - 1);
    if(readed < 1)
        return false;

    close(fd);

    if(value[readed - 1] == '\n')
        value[readed - 1] = '\0';
    else
        value[readed] = '\0';

    trimTrailingWhitespaces(value);

    ffPrintLogoAndKey(instance, key);
    puts(value);

    return true;
}

static void parseLine(FFinstance* instance, const char* line)
{
    if(strcasecmp(line, "break") == 0)
        ffPrintBreak(instance);
    else if(strcasecmp(line, "title") == 0)
        ffPrintTitle(instance);
    else if(strcasecmp(line, "seperator") == 0)
        ffPrintSeperator(instance);
    else if(strcasecmp(line, "os") == 0)
        ffPrintOS(instance);
    else if(strcasecmp(line, "host") == 0)
        ffPrintHost(instance);
    else if(strcasecmp(line, "kernel") == 0)
        ffPrintKernel(instance);
    else if(strcasecmp(line, "uptime") == 0)
        ffPrintUptime(instance);
    else if(strcasecmp(line, "packages") == 0)
        ffPrintPackages(instance);
    else if(strcasecmp(line, "shell") == 0)
        ffPrintShell(instance);
    else if(strcasecmp(line, "resolution") == 0)
        ffPrintResolution(instance);
    else if(strcasecmp(line, "desktopenvironment") == 0 || strcasecmp(line, "de") == 0)
        ffPrintDesktopEnvironment(instance);
    else if(strcasecmp(line, "theme") == 0)
        ffPrintTheme(instance);
    else if(strcasecmp(line, "icons") == 0)
        ffPrintIcons(instance);
    else if(strcasecmp(line, "font") == 0)
        ffPrintFont(instance);
    else if(strcasecmp(line, "terminal") == 0)
        ffPrintTerminal(instance);
    else if(strcasecmp(line, "cpu") == 0)
        ffPrintCPU(instance);
    else if(strcasecmp(line, "gpu") == 0)
        ffPrintGPU(instance);
    else if(strcasecmp(line, "memory") == 0)
        ffPrintMemory(instance);
    else if(strcasecmp(line, "disk") == 0)
        ffPrintDisk(instance);
    else if(strcasecmp(line, "battery") == 0)
        ffPrintBattery(instance);
    else if(strcasecmp(line, "locale") == 0)
        ffPrintLocale(instance);
    else if(strcasecmp(line, "colors") == 0)
        ffPrintColors(instance);
    else
        ffPrintError(instance, line, "<no implementaion provided>");
}

static void parseStructureFile(FFinstance* instance)
{
    char configFolder[256];
    strcpy(configFolder, instance->state.passwd->pw_dir);
    strcat(configFolder, "/.config");

    mkdir(configFolder, S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH); //I hope everybody has a config folder but whow knews
    
    strcat(configFolder, "/fastfetch/");
    mkdir(configFolder, S_IRWXU | S_IRGRP | S_IROTH);
    
    char valueDir[256];
    strcpy(valueDir, configFolder);
    strcat(valueDir, "values/");
    mkdir(valueDir, S_IRWXU | S_IRGRP | S_IROTH);

    char structureFileName[256];
    strcpy(structureFileName, configFolder);
    strcat(structureFileName, "structure");

    FILE* structureFile;
    if(access(structureFileName, F_OK) != 0)
        structureFile = createStructureFile(structureFileName);
    else
        structureFile = fopen(structureFileName, "r");

    char* line = NULL;
    size_t len;
    ssize_t read;

    while ((read = getline(&line, &len, structureFile)) != -1)
    {
        if(line[read - 1] == '\n')
            line[read - 1] = '\0';

        trimTrailingWhitespaces(line);
        
        if(!customValue(instance, line, valueDir))
            parseLine(instance, line);
    }

    free(line);
    fclose(structureFile);
}

int main(int argc, char** argv)
{
    FFinstance instance;
    ffInitState(&instance.state);
    defaultConfig(&instance.config);
    parseArguments(argc, argv, &instance.config);
    parseStructureFile(&instance);
    return 0;
}
