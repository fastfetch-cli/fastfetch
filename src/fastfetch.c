#include "fastfetch.h"
#include "fastfetch_config.h"
#include "util/FFvaluestore.h"

#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FASTFETCH_DEFAULT_STRUCTURE "Title:Seperator:OS:Host:Kernel:Uptime:Packages:Shell:Resolution:DE:WM:Theme:Icons:Font:Terminal:TerminalFont:CPU:GPU:Memory:Disk:Battery:Locale:Break:Colors"

#define FASTFETCH_DEFAULT_CONFIG \
    "## Fastfetch configuration\n" \
    "## Put arguments here to make them permanently.\n" \
    "## Direct arguments will overwrite the corresponding ones in this files\n" \
    "## Each line is whitespace trimmed on beginn and end.\n" \
    "## Empty lines or lines starting with # are ignored.\n" \
    "## There are more arguments possible than listed here, take a look at fastfetch --help!\n" \
    "## This version of the file was shipped with "FASTFETCH_PROJECT_VERSION".\n" \
    "## Use fastfetch --print-default-config > ~/.config/fastfetch/config.conf to generate a new one with current defaults.\n" \
    "\n" \
    "## General options:\n" \
    "# --structure "FASTFETCH_DEFAULT_STRUCTURE"\n" \
    "# --spacer 4\n" \
    "# --seperator \": \"\n" \
    "# --offsetx 0\n" \
    "# --recache false\n" \
    "# --show-errors false\n" \
    "\n" \
    "## Logo options:\n" \
    "# --color-logo true\n" \
    "\n" \
    "## OS options:\n" \
    "# --os-architecture true\n" \
    "\n" \
    "## Host options:\n" \
    "# --host-version true\n" \
    "\n" \
    "## Kernel options:\n" \
    "# --kernel-release true\n" \
    "# --kernel-version false\n" \
    "\n" \
    "## Packages options:\n" \
    "# --packages-combined false\n" \
    "# --packages-combined-names true\n" \
    "# --packages-pacman true\n" \
    "# --packages-flatpak true\n" \
    "\n" \
    "## Shell options:\n" \
    "# --shell-path false\n" \
    "\n" \
    "## Resolution options:\n" \
    "# --resolution-refreshrate true\n" \
    "# --resolution-libX11 libX11.so\n"\
    "# --resolution-libXrandr libXrandr.so\n" \
    "\n" \
    "## Battery options:\n" \
    "# --battery-manufacturer true\n" \
    "# --battery-model true\n" \
    "# --battery-technology true\n" \
    "# --battery-capacity true\n" \
    "# --battery-status true\n"

//Things only needed by fastfetch
typedef struct FFdata
{
    FFvaluestore valuestore;
    char structure[2048];
    char logoName[32];
} FFdata;

static inline void printHelp()
{
    puts(
        "Usage: fastfetch <options>\n"
        "\n"
        "Informative options:\n"
        "   -h,           --help:                 shows this message and exits\n"
        "   -h <command>, --help <command>:       shows help for a specific command and exits\n"
        "   -v            --version:              prints the version of fastfetch and exits\n"
        "                 --list-logos:           list available logos and exits\n"
        "                 --print-logos:          shows available logos and exits\n"
        "                 --print-default-config: prints the default config and exits\n"
        "\n"
        "General options:\n"
        "                --structure <structure>: sets the structure of the fetch. Must be a colon seperated list of keys\n"
        "                --set <key=value>:       hard set the value of an key\n"
        "   -c <color>,  --color <color>:         sets the color of the keys. Must be a linux console color code (+)\n"
        "                --spacer <width>:        sets the distance between logo and text\n"
        "   -s <str>,    --seperator <str>:       sets the seperator between key and value. Default is a colon with a space\n"
        "   -x <offset>, --offsetx <offset>:      sets the x offset. Can be negative to cut the logo, but no more than logo width.\n"
        "                --show-errors <?value>:  print occuring errors\n"
        "   -r <?value>  --recache <?value>:      if set to true, no cached values will be used\n"
        "\n"
        "Logo options:\n"
        "   -l <name>, --logo <name>:         sets the shown logo. Also changes the main color accordingly\n"
        "              --color-logo <?value>: if set to false, the logo will be black / white\n"
        "\n"
        "OS options:\n"
        "   --os-architecture <?value>: Show the architecture of the os\n"
        "   --os-format <format>:       Provide the printf format string for os output (+)\n"
        "\n"
        "Host options:\n"
        "   --host-version <?value>: Show the version of the host platform, if possible. Most likely, this will be the BIOS version\n"
        "   --host-format <format>:  Provide the printf format string for host output (+)\n"
        "\n"
        "Kernel options:\n"
        "   --kernel-release <?value>: Shows the release of the kernel\n"
        "   --kernel-version <?value>: Shows the build version of the kernel\n"
        "\n"
        "Packages options:\n"
        "   --packages-combined <?value>:       Show the sum of all packages\n"
        "   --packages-combined-names <?value>: Show the names of the package managers after the sum if in packages-combined mode\n"
        "   --packages-pacman <?value>:         Count pacman packages\n"
        "   --packages-flatpak <?value>:        Count flatpak packages\n"
        "   --packages-format <format>:         Provide the printf format string for packages output (+)\n"
        "\n"
        "Shell options:\n"
        "    --shell-path <?value>: Show the full path of the shell\n"
        "\n"
        "Resolution optins:\n"
        "   --resolution-refreshrate <?value>: Show the refresh rate of the monitor\n"
        "   --resolution-libX11 <path>:        Set the path to the x11 library to load\n"
        "   --resolution-libXrandr <path>:     Set the path to the xrandr library to load\n"
        "   --resolution-format <format>:      Provide the printf format string for resolution output (+)\n"
        "\n"
        "Battery options:\n"
        "   --battery-manufacturer <?value>: Show the manufacturer of the battery, if possible\n"
        "   --battery-model <?value>:        Show the model of the battery, if possible\n"
        "   --battery-technology <?value>:   Show the technology of the battery, if possible\n"
        "   --battery-capacity <?value>:     Show the capacity of the battery, if possible\n"
        "   --battery-status <?value>:       Show the status of the battery, if possible\n"
        "   --battery-format <format>:       Provide the printf format string for battery output (+)\n"
        "\n"
        "If an value starts with an ?, it is optional. \"true\" will be used if not set.\n"
        "An (+) at the end indicates that more help can be printed with --help <option>\n"
        "All options can be make permanent in $XDG_CONFIG_HOME/fastfetch/config.conf"
    );
}

static inline void printCommandHelpColor()
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

static inline void printCommandHelpOsFormat()
{
    puts(
        "usage fastfetch --os-format <format>\n"
        "\n"
        "<format> is a string of maximum length 32, which is passed to printf as the format string.\n"
        "The arguments passed to printf are 2 strings in following order:\n"
        "name, architecture\n"
        "If an value could not be determined, it will be an zero length string.\n"
        "The default value is something like \"%s %s\"."
    );
}

static inline void printCommandHelpHostFormat()
{
    puts(
        "usage fastfetch --host-format <format>\n"
        "\n"
        "<format> is a string of maximum length 32, which is passed to printf as the format string.\n"
        "The arguments passed to printf are 3 strings in following order:\n"
        "family, name, version\n"
        "If an value could not be determined, it will be an zero length string.\n"
        "The default value is something like \"%s %s %s\"."
    );
}

static inline void printCommandHelpBatteryFormat()
{
    puts(
        "usage fastfetch --battery-format <format>\n"
        "\n"
        "<format> is a string of maximum length 32, which is passed to printf as the format string.\n"
        "The arguments passed to printf are 5 strings in following order:\n"
        "manufacturer, model, technology, capacity, status\n"
        "If an value was disabled via battery-* argument, or could not be determined, it will be an zero length string.\n"
        "The default value is something like \"%s %s (%s) [%s; %s]\"."
    );
}

static inline void printCommandHelpPackagesFormat()
{
    puts(
        "usage: fastfetch --packages-format <format>\n"
        "\n"
        "<format> is a string of maximum length 32, which is passed to printf as the format string.\n"
        "if --packages-combined is set to false, the numbers of packages are passed to printf in following order as uint32_t:\n"
        "all, pacman, flatpak, all\n"
        "if an value is disabled via a packages-* argument, or could not be determined, zero is passed\n"
        "The default value is something like \"%.0s%u (pacman), %u (flatpack)\""
    );
}

static inline void printCommandHelpResolutionFormat()
{
    puts(
        "usage: fastfetch --resolution-format <format>\n"
        "\n"
        "<format> is a string of maximum length 32, which is passed to printf as the format string.\n"
        "the values passed to printf are in following order:\n"
        "width (int), height (int), refreshRate (short)\n"
        "if refresh rate is disabled, or could not be determined, zero is passed\n"
        "The default value is something like \"%ix%i @ %dHz\""
    );
}

static inline void printCommandHelp(const char* command)
{
    if(strcasecmp(command, "c") == 0 || strcasecmp(command, "color") == 0)
        printCommandHelpColor();
    else if(strcasecmp(command, "os-format") == 0)
        printCommandHelpOsFormat();
    else if(strcasecmp(command, "host-format") == 0)
        printCommandHelpHostFormat();
    else if(strcasecmp(command, "battery-format") == 0)
        printCommandHelpBatteryFormat();
    else if(strcasecmp(command, "packages-format") == 0)
        printCommandHelpPackagesFormat();
    else if(strcasecmp(command, "resolution-format") == 0)
        printCommandHelpResolutionFormat();
    else
        printf("No specific help for command %s provided\n", command);
}

static inline bool optionParseBoolean(const char* str)
{
    if(str == NULL)
        return true;

    return (
        strcasecmp(str, "true") == 0 ||
        strcasecmp(str, "yes")  == 0 ||
        strcasecmp(str, "1")    == 0
    );
}

static inline void optionParseString(const char* key, const char* value, char* target, uint32_t capacity)
{
    if(value == NULL)
    {
        printf("Error: usage: %s <str>\n", key);
        exit(477);
    }
    size_t len = strlen(value);
    if(len > capacity)
    {
        printf("max string length for %s is %u, %zu given\n", key, capacity, len);
        exit(478);
    }
    strcpy(target, value);
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
    else if(strcasecmp(line, "windowmanager") == 0 || strcasecmp(line, "wm") == 0)
        ffPrintWM(instance);
    else if(strcasecmp(line, "theme") == 0)
        ffPrintTheme(instance);
    else if(strcasecmp(line, "icons") == 0)
        ffPrintIcons(instance);
    else if(strcasecmp(line, "font") == 0)
        ffPrintFont(instance);
    else if(strcasecmp(line, "terminal") == 0)
        ffPrintTerminal(instance);
    else if(strcasecmp(line, "terminalfont") == 0)
        ffPrintTerminalFont(instance);
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
        puts(FASTFETCH_PROJECT_NAME" "FASTFETCH_PROJECT_VERSION);
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
    else if(strcasecmp(key, "-c") == 0 || strcasecmp(key, "--color") == 0)
    {
        if(value == NULL)
        {
            printf("Error: usage: %s <color>\n", key);
            exit(402);
        }
        size_t len = strlen(value);
        if(len > 28)
        {
            printf("Error: max color string length is 28, %zu given\n", len);
            exit(403);
        }
        sprintf(instance->config.color, "\033[%sm", value);
    }
    else if(strcasecmp(key, "--spacer") == 0)
    {
        if(value == NULL)
        {
            printf("Error: usage: %s <width>\n", key);
            exit(404);
        }
        if(sscanf(value, "%hd", &instance->config.logo_spacer) != 1)
        {
            printf("Error: couldn't parse %s to uint16_t\n", value);
            exit(405);
        }
    }
    else if(strcasecmp(key, "-x") == 0 || strcasecmp(key, "--offsetx") == 0)
    {
        if(value == NULL)
        {
            printf("Error: usage: %s <offset>\n", key);
            exit(408);
        }
        if(sscanf(value, "%hi", &instance->config.offsetx) != 1)
        {
            printf("Error: couldn't parse %s to int16_t\n", value);
            exit(409);
        }
    }
    else if(strcasecmp(key, "--set") == 0)
    {
        if(value == NULL)
        {
            printf("Error: usage: %s <key=value>\n", key);
            exit(411);
        }

        char* seperator = strchr(value, '=');

        if(seperator == NULL)
        {
            printf("Error: usage: %s <key=value>, '=' missing\n", key);
            exit(412);
        }

        *seperator = '\0';

        ffValuestoreSet(&data->valuestore, value, seperator + 1);
    }
    else if(strcasecmp(key, "-r") == 0 || strcasecmp(key, "--recache") == 0)
    {
        //Set cacheSave as well, beacuse the user expects the values to  be cached when expliciting using --recache   
        instance->config.recache = optionParseBoolean(value);
        instance->config.cacheSave = instance->config.recache;
    }
    else if(strcasecmp(key, "--structure") == 0)
        optionParseString(key, value, data->structure, sizeof(data->structure));
    else if(strcasecmp(key, "-l") == 0 || strcasecmp(key, "--logo") == 0)
        optionParseString(key, value, data->logoName, sizeof(instance->config.logo));
    else if(strcasecmp(key, "-s") == 0 || strcasecmp(key, "--seperator") == 0)
        optionParseString(key, value, instance->config.seperator, sizeof(instance->config.seperator));
    else if(strcasecmp(key, "--show-errors") == 0)
        instance->config.showErrors = optionParseBoolean(value);
    else if(strcasecmp(key, "--color-logo") == 0)
        instance->config.colorLogo = optionParseBoolean(value);
    else if(strcasecmp(key, "--os-architecture") == 0)
        instance->config.osShowArchitecture = optionParseBoolean(value);
    else if(strcasecmp(key, "--os-format") == 0)
        optionParseString(key, value, instance->config.osFormat, sizeof(instance->config.osFormat));
    else if(strcasecmp(key, "--host-version") == 0)
        instance->config.hostShowVersion = optionParseBoolean(value);
    else if(strcasecmp(key, "--host-format") == 0)
        optionParseString(key, value, instance->config.hostFormat, sizeof(instance->config.hostFormat));
    else if(strcasecmp(key, "--kernel-release") == 0)
        instance->config.kernelShowRelease = optionParseBoolean(value);
    else if(strcasecmp(key, "--kernel-version") == 0)
        instance->config.kernelShowVersion = optionParseBoolean(value);
    else if(strcasecmp(key, "--packages-combined") == 0)
        instance->config.packagesCombined = optionParseBoolean(value);
    else if(strcasecmp(key, "--packages-combined-names") == 0)
    {
        instance->config.packagesCombined = true;
        instance->config.packagesCombinedNames = optionParseBoolean(value);
    }
    else if(strcasecmp(key, "--packages-pacman") == 0)
        instance->config.packagesShowPacman = optionParseBoolean(value);
    else if(strcasecmp(key, "--packages-flatpak") == 0)
        instance->config.packagesShowFlatpak = optionParseBoolean(value);
    else if(strcasecmp(key, "--packages-format") == 0)
        optionParseString(key, value, instance->config.packagesFormat, sizeof(instance->config.packagesFormat));
    else if(strcasecmp(key, "--shell-path") == 0)
        instance->config.shellShowPath = optionParseBoolean(value);
    else if(strcasecmp(key, "--resolution-refreshrate") == 0)
        instance->config.resolutionShowRefreshRate = optionParseBoolean(value);
    else if(strcasecmp(key, "--resolution-libX11") == 0)
        optionParseString(key, value, instance->config.resolutionLibX11, sizeof(instance->config.resolutionLibX11));
    else if(strcasecmp(key, "--resolution-libXrandr") == 0)
        optionParseString(key, value, instance->config.resolutionLibXrandr, sizeof(instance->config.resolutionLibXrandr));
    else if(strcasecmp(key, "--resolution-format") == 0)
        optionParseString(key, value, instance->config.resolutionFormat, sizeof(instance->config.resolutionFormat));
    else if(strcasecmp(key, "--battery-manufacturer") == 0)
        instance->config.batteryShowManufacturer = optionParseBoolean(value);
    else if(strcasecmp(key, "--battery-model") == 0)
        instance->config.batteryShowModel = optionParseBoolean(value);
    else if(strcasecmp(key, "--battery-technology") == 0)
        instance->config.batteryShowTechnology = optionParseBoolean(value);
    else if(strcasecmp(key, "--battery-capacity") == 0)
        instance->config.batteryShowCapacity = optionParseBoolean(value);
    else if(strcasecmp(key, "--battery-status") == 0)
        instance->config.batteryShowStatus = optionParseBoolean(value);
    else if(strcasecmp(key, "--battery-format") == 0)
        optionParseString(key, value, instance->config.batteryFormat, sizeof(instance->config.batteryFormat));
    else
    {
        printf("Error: unknown option: %s\n", key);
        exit(400);
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

    char* lineStart = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&lineStart, &len, file)) != -1) {

        //We need to copy lineStart because we modify this value, but need the original for free
        char* line = lineStart;

        if(line[read - 1] == '\n')
            line[read - 1] = '\0';
        else
            line[read] = '\0';

        ffTrimTrailingWhitespace(line);

        //This trims leading whitespace
        while(*line == ' ')
            ++line;

        if(line[0] == '\0' || line[0] == '#')
            continue;

        char* valueStart = strchr(line, ' ');
        if(valueStart == NULL)
        {
            parseOption(instance, data, line, NULL);
        }
        else
        {
            //Seperate key and value by simply replacing the first space with a \0
            *valueStart = '\0';
            ++valueStart;

            //Trim whitespace at beginn of value
            while(*valueStart == ' ')
                ++valueStart;

            //If we want whitespace in values, we need to quote it. This is done to keep consistency with shell.
            if(*valueStart == '"')
            {
                char* last = valueStart + strlen(valueStart) - 1;
                if(*last == '"')
                {
                    ++valueStart;
                    *last = '\0';
                }
            }

            parseOption(instance, data, line, valueStart);
        }
    }

    if(lineStart != NULL)
        free(lineStart);

    fclose(file);
}

static void parseArguments(FFinstance* instance, FFdata* data, int argc, const char** argv)
{
    //This is generally a good idea, because cached values most likely contain values generated with other arguments
    //Hovwever we dont do this with arguments in the config file, because they are more likely to stay the same
    //If caching is _really_ wanted (e.g. a call in .bashrc with arguments), one can still set --recache false
    if(argc > 1)
    {
        instance->config.recache = true;
        instance->config.cacheSave = false;
    }

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
    ffDefaultConfig(&instance.config);

    FFdata data;
    ffValuestoreInit(&data.valuestore);
    data.structure[0] = '\0'; //We use this in run to detect if a structure was set
    data.logoName[0] = '\0';  //We use this in applyData to detect if a logo was set

    parseConfigFile(&instance, &data);
    parseArguments(&instance, &data, argc, argv);
    applyData(&instance, &data); //Here we do things that need to be done after parsing all options

    run(&instance, &data);

    ffValuestoreDelete(&data.valuestore);
}
