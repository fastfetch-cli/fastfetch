#include "fastfetch.h"
#include "fastfetch_config.h"
#include "util/FFvaluestore.h"

#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FASTFETCH_DEFAULT_STRUCTURE "Title:Seperator:OS:Host:Kernel:Uptime:Packages:Shell:Resolution:DE:Theme:Icons:Font:Terminal:CPU:GPU:Memory:Disk:Battery:Locale:Break:Colors"

#define FASTFETCH_DEFAULT_CONFIG \
    "--structure "FASTFETCH_DEFAULT_STRUCTURE"\n" \
    "--seperator 4\n" \
    "--offsetx 0\n" \
    "--color-logo true\n" \
    "--recache false\n" \
    "--show-errors false\n"

//Things only needed by fastfetch
typedef struct FFdata
{
    FFvaluestore valuestore;
    char structure[2048];
    char logoName[32];
} FFdata;

static void printHelp()
{
    puts(
        "usage: fastfetch <options>\n"
        "\n"
        "   -h,           --help:                  shows this message and exits\n"
        "   -h <command>, --help <command>:        shows help for a specific command and exits\n"
        "   -v            --version:               prints the version of fastfetch and exits\n"
        "                 --list-logos:            list available logos and exits\n"
        "                 --print-logos:           shows available logos and exits\n"
        "                 --print-default-config:  prints the default config and exits\n"
        "                 --structure <structure>: sets the structure of the fetch. Must be a colon seperated list of keys\n"
        "                 --set <key=value>:       hard set the value of an key"
        "   -l <name>,    --logo <name>:           sets the shown logo. Also changes the main color accordingly\n"
        "   -c <color>,   --color <color>:         sets the color of the keys. Must be a linux console color code\n"
        "   -s <width>,   --seperator <width>:     sets the distance between logo and text\n"
        "   -x <offset>,  --offsetx <offset>:      sets the x offset. Can be negative to cut the logo, but no more than logo width.\n"
        "                 --show-errors <?value>:  print occuring errors\n"
        "                 --color-logo <?value>:   if set to false, the logo will be black / white\n"
        "   -r <?value>   --recache <?value>:      if set to true, no cached values will be used\n"
        "\n"
        "If an value starts with an ?, it is optional. \"true\" will be used if not set.\n"
        "\n"
        "All options can be make permanent in $XDG_CONFIG_HOME/fastfetch/config.conf"
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

static void defaultConfig(FFconfig* config)
{
    config->color[0] = '\0';
    config->logo_seperator = 4;
    config->offsetx = 0;
    config->titleLength = 20; // This is overwritten by ffPrintTitle
    config->colorLogo = true;
    config->showErrors = false;
    config->recache = false;
}

static bool parseBoolean(const char* str)
{
    return
        strcasecmp(str, "true") == 0 ||
        strcasecmp(str, "yes")  == 0 ||
        strcasecmp(str, "1")    == 0
    ;
}

static void parseStructureCommand(FFinstance* instance, FFdata* data, const char* line)
{
    const char* setValue = ffValuestoreGet(&data->valuestore, line);
    if(setValue != NULL)
    {
        ffPrintCustom(instance, line, setValue);
        return;
    }

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

static void parseOption(FFinstance* instance, FFdata* data, const char* key, const char* value)
{
    if(strcasecmp(key, "-h") == 0 || strcasecmp(key, "--help") == 0)
    {
        if(value == NULL)
            printHelp();
        else
            printCommandHelp(value);

        exit(0);
    }
    else if(strcasecmp(key, "-v") == 0 || strcasecmp(key, "--version") == 0)
    {
        puts(FASTFETCH_PROJECT_NAME" "FASTFETCH_PROJECT_VER);
        exit(0);
    }
    else if(strcasecmp(key, "--list-logos") == 0)
    {
        ffListLogos();
        exit(0);
    }
    else if(strcasecmp(key, "--print-logos") == 0)
    {
        ffPrintLogos(instance->config.colorLogo);
        exit(0);
    }
    else if(strcasecmp(key, "--print-default-config") == 0)
    {
        puts(FASTFETCH_DEFAULT_CONFIG);
        exit(0);
    }
    else if(strcasecmp(key, "--show-errors") == 0)
    {
        if(value == NULL)
            instance->config.showErrors = true;
        else
            instance->config.showErrors = parseBoolean(value);
    }
    else if(strcasecmp(key, "-l") == 0 || strcasecmp(key, "--logo") == 0)
    {
        if(value == NULL)
        {
            printf("Error: usage: %s <logo>\n", key);
            exit(41);
        }

        strcpy(data->logoName, value);
    }
    else if(strcasecmp(key, "--color-logo") == 0)
    {
        if(value == NULL)
            instance->config.colorLogo = true;
        else
            instance->config.colorLogo = parseBoolean(value);
    }
    else if(strcasecmp(key, "-c") == 0 || strcasecmp(key, "--color") == 0)
    {
        if(value == NULL)
        {
            printf("Error: usage: %s <color>\n", key);
            exit(42);
        }
        size_t len = strlen(value);
        if(len > 28)
        {
            printf("Error: max color string length is 28, %zu given\n", len);
            exit(43);
        }
        sprintf(instance->config.color, "\033[%sm", value);
    }
    else if(strcasecmp(key, "-s") == 0 || strcasecmp(key, "--seperator") == 0)
    {
        if(value == NULL)
        {
            printf("Error: usage: %s <width>\n", key);
            exit(44);
        }
        if(sscanf(value, "%hd", &instance->config.logo_seperator) != 1)
        {
            printf("Error: couldn't parse %s to uint16_t\n", value);
            exit(45);
        }
    }
    else if(strcasecmp(key, "-x") == 0 || strcasecmp(key, "--offsetx") == 0)
    {
        if(value == NULL)
        {
            printf("Error: usage: %s <offset>\n", key);
            exit(46);
        }
        if(sscanf(value, "%hi", &instance->config.offsetx) != 1)
        {
            printf("Error: couldn't parse %s to int16_t\n", value);
            exit(47);
        }
    }
    else if(strcasecmp(key, "--structure") == 0)
    {
        if(value == NULL)
        {
            printf("Error: usage: %s <structure>\n", key);
            exit(46);
        }
        strcpy(data->structure, value);
    }
    else if(strcasecmp(key, "--set") == 0)
    {
        if(value == NULL)
        {
            printf("Error: usage: %s <key=value>\n", key);
            exit(47);
        }

        char* seperator = strchr(value, '=');
        *seperator = '\0';

        ffValuestoreSet(&data->valuestore, value, seperator + 1);
    }
    else if(strcasecmp(key, "-r") == 0 || strcasecmp(key, "--recache") == 0)
    {
        if(value == NULL)
            instance->config.recache = true;
        else
            instance->config.recache = parseBoolean(value);
    }
    else
    {
        printf("Error: unknown option: %s\n", key);
        exit(40);
    }
}

static void parseConfigFile(FFinstance* instance, FFdata* data)
{
    char fileName[256];

    const char* xdgConfig = getenv("XDG_CONFIG_HOME");
    if(xdgConfig == NULL)
    {
        strcpy(fileName, instance->state.passwd->pw_dir);
        strcat(fileName, "/.config");
    }
    else
    {
        strcpy(fileName, xdgConfig);
    }
    mkdir(fileName, S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH); //I hope everybody has a config folder but whow knews
    
    strcat(fileName, "/fastfetch/");
    mkdir(fileName, S_IRWXU | S_IRGRP | S_IROTH);

    strcat(fileName, "config.conf");

    if(access(fileName, F_OK) != 0)
    {
        FILE* file = fopen(fileName, "w");
        fputs(FASTFETCH_DEFAULT_CONFIG, file);
        fclose(file);
        return;
    }
    
    FILE* file = fopen(fileName, "r");

    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {

        if(line[read - 1] == '\n')
            line[read - 1] = '\0';
        else
            line[read] = '\0';

        ffTrimTrailingWhitespace(line);
        if(line[0] == '\0')
            return;

        char* firstIndex = strchr(line, ' ');   
    
        if(firstIndex == NULL)
        {
            parseOption(instance, data, line, NULL);
        }
        else
        {
            *firstIndex = '\0';
            parseOption(instance, data, line, firstIndex + 1);
        }
    }

    if(line)
        free(line);

    fclose(file);
}

static void parseArguments(FFinstance* instance, FFdata* data, int argc, const char** argv)
{
    for(int i = 1; i < argc; i++)
    {
        if(i == argc - 1 || argv[i + 1][0] == '-')
        {
            parseOption(instance, data, argv[i], NULL);
        }
        else
        {
            parseOption(instance, data, argv[i], argv[i + 1]);
            ++i;
        }
    }
}

static void applyData(FFinstance* instance, FFdata* data)
{
    //We must do this after parsing all options because of color options
    if(data->logoName[0] == '\0')
        ffLoadLogo(&instance->config);
    else
        ffLoadLogoSet(&instance->config, data->logoName);

    //This must be done after loading the logo
    if(instance->config.color[0] == '\0')
        strcpy(instance->config.color, instance->config.logo.color);
}

static void run(FFinstance* instance, FFdata* data)
{
    if(data->structure[0] == '\0')
        strcpy(data->structure, FASTFETCH_DEFAULT_STRUCTURE);
    
    char* remaining = data->structure;
    char* colon = NULL;

    while(true)
    {
        colon = strchr(remaining, ':');
        
        if(colon != NULL)
            *colon = '\0';
        
        parseStructureCommand(instance, data, remaining);
        
        if(colon != NULL)
            remaining = colon + 1;
        else
            break;
    }
}

int main(int argc, const char** argv)
{
    FFinstance instance;
    ffInitState(&instance.state);
    defaultConfig(&instance.config);

    FFdata data;
    ffValuestoreInit(&data.valuestore);
    data.structure[0] = '\0'; //We use this in run to detect if a structure was set
    data.logoName[0] = '\0';

    parseConfigFile(&instance, &data);
    parseArguments(&instance, &data, argc, argv);
    applyData(&instance, &data);

    run(&instance, &data);

    ffValuestoreDelete(&data.valuestore);
}
