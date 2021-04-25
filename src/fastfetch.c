#include "fastfetch.h"
#include "util/FFvaluestore.h"

#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FASTFETCH_DEFAULT_STRUCTURE "Title:Seperator:OS:Host:Kernel:Uptime:Packages:Shell:Resolution:DE:WM:WMTheme:Theme:Icons:Font:Terminal:TerminalFont:CPU:GPU:Memory:Disk:Battery:Locale:Break:Colors"

#define FASTFETCH_DEFAULT_CONFIG \
    "# Fastfetch configuration\n" \
    "# Put arguments here to make them permanently (one per line). \n" \
    "# Direct arguments will overwrite the corresponding ones in this files\n" \
    "# Each line is whitespace trimmed on beginn and end.\n" \
    "# Empty lines or lines starting with # are ignored.\n" \
    "# There are more arguments possible than listed here, take a look at fastfetch --help!\n" \
    "# This version of the file was shipped with "FASTFETCH_PROJECT_VERSION".\n" \
    "# Use fastfetch --print-default-config > ~/.config/fastfetch/config.conf to generate a new one with current defaults.\n"

//Things only needed by fastfetch
typedef struct FFdata
{
    FFvaluestore valuestore;
    FFstrbuf structure;
    FFstrbuf logoName;
    bool multithreading;
} FFdata;

static inline void printHelp()
{
    puts(
        "Usage: fastfetch <options>\n"
        "\n"
        "Informative options:\n"
        "   -h,           --help:                    shows this message and exits\n"
        "   -h <command>, --help <command>:          shows help for a specific command and exits\n"
        "   -v            --version:                 prints the version of fastfetch and exits\n"
        "                 --list-logos:              list available logos and exits\n"
        "                 --print-logos:             shows available logos and exits\n"
        "                 --print-default-config:    prints the default config and exits\n"
        "                 --print-default-structure: prints the default stucture and exits\n"
        "                 --print-available-modules: prints a list of available modules and exits\n"
        "\n"
        "General options:\n"
        "                --structure <structure>:         sets the structure of the fetch. Must be a colon seperated list of keys. Use --print-available-modules to show the default\n"
        "                --set <key=value>:               hard set the value of an key\n"
        "   -c <color>,  --color <color>:                 sets the color of the keys. Must be a linux console color code (+)\n"
        "                --spacing <width>:               sets the distance between logo and text\n"
        "   -s <str>,    --seperator <str>:               sets the seperator between key and value. Default is a colon with a space\n"
        "   -x <offset>, --offsetx <offset>:              sets the x offset. Can be negative to cut the logo, but no more than logo width.\n"
        "                --show-errors <?value>:          print occuring errors\n"
        "   -r <?value>  --recache <?value>:              generate new cached values\n"
        "                --nocache <?value>:              dont use cached values, but also dont overwrite existing ones\n"
        "                --print-remaining-logo <?value>: print the remaining logo, if it is higher than the number of lines shown\n"
        "                --multithreading <?value>:       use multiple threads to calculate values\n"
        "\n"
        "Logo options:\n"
        "   -l <name>, --logo <name>:         sets the shown logo. Also changes the main color accordingly\n"
        "              --color-logo <?value>: if set to false, the logo will be black / white\n"
        "\n"
        "Format options: Provide the format string for custom output (+)\n"
        "   --os-format <format>\n"
        "   --host-format <format>\n"
        "   --kernel-format <format>\n"
        "   --uptime-format <format>\n"
        "   --packages-format <format>\n"
        "   --shell-format <format>\n"
        "   --resolution-format <format>\n"
        "   --de-format <format>\n"
        "   --wm-format <format>\n"
        "   --wm-theme-format <format>\n"
        "   --theme-format <format>\n"
        "   --icons-format <format>\n"
        "   --font-format <format>\n"
        "   --terminal-format <format>\n"
        "   --terminal-font-format <format>\n"
        "   --cpu-format <format>\n"
        "   --gpu-format <format>\n"
        "   --memory-format <format>\n"
        "   --disk-format <format>\n"
        "   --battery-format <format>\n"
        "   --locale-format <format>\n"
        "\n"
        "Key options: Provide a custom key for an output\n"
        "   --os-key <key>\n"
        "   --host-key <key>\n"
        "   --kernel-key <key>\n"
        "   --uptime-key <key>\n"
        "   --packages-key <key>\n"
        "   --shell-key <key>\n"
        "   --resolution-key <key>: takes the resolution index as argument\n"
        "   --de-key <key>\n"
        "   --wm-key <key>\n"
        "   --wm-theme-key <key>\n"
        "   --theme-key <key>\n"
        "   --icons-key <key>\n"
        "   --font-key <key>\n"
        "   --terminal-key <key>\n"
        "   --terminal-font-key <key>\n"
        "   --cpu-key <key>\n"
        "   --gpu-key <key>: takes the gpu index as format argument\n"
        "   --memory-key <key>\n"
        "   --disk-key <key>: takes the mount path as format argument\n"
        "   --battery-key <key>: takes the battery index as format argument\n"
        "   --locale-key <key>\n"
        "\n"
        "Library optins: Set the path of a library to load\n"
        "   --lib-PCI <path>\n"
        "   --lib-X11 <path>\n"
        "   --lib-Xrandr <path>\n"
        "   --lib-DConf <path>\n"
        "   --lib-wayland <path>\n"
        "\n"
        "Module specific options:\n"
        "   --disk-folders <folders>: A colon seperated list of folder paths for the disk output. Default is \"/:/home\"\n"
        "   --battery-dir <folder>:   The directory were the battery folders are in. Standard: /sys/class/power_supply/\n"
        "\n"
        "Parsing is not case sensetive. E.g. \"--lib-PCI\" is equal to \"--Lib-Pci\"\n"
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
        "Infos about them can be found here: https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_(Select_Graphic_Rendition)_parameters.\n"
        "Examples:\n"
        "   \"--color 35\":    sets the color to pink\n"
        "   \"--color 4;92\":  sets the color to bright Green with underline\n"
        "   \"--color 5;104\": blinking text on a blue background\n"
        "If no color is set, the main color of the logo will be used.\n"
    );
}

static inline void printCommandHelpFormat()
{
    puts(
        "A format string is string that contains placeholder for values.\n"
        "These placeholders beginn with a '{', contain the index of the value and end with a '}'.\n"
        "For example the format string \"Values: {1} ({2})\", with the values \"First\" and \"My second val\", will produce \"Values: First (My second val)\".\n"
        "The format string can contain placeholdes in any oder and multiple occurences.\n"
        "To include spaces when setting from command line, surround the whole string with double quotes (\").\n"
        "\n"
        "If the value index is missing, meaning the placeholder is \"{}\", an internal counter sets the value index.\n"
        "This means, that the format string \"Values: {1} ({2})\" is equivalent to \"Values: {} ({})\".\n"
        "Note that this counter only counts empty placeholders, so the format string \"{2} {} {}\" will contain the second value, then the first, and then again the second.\n"
        "\n"
        "To make formatting easier, a double open curly brace (\"{{\") will be printed as a single open curly brace and not counted as the beginn of a placeholder.\n"
        "If a value index is missformatted or wants a non existing value, it will be printed as is, with the curly braces around it.\n"
        "If the last placeholder isn't closed, it will be treated like it was at the end of the format string.\n"
        "\n"
        "To only print something if a variable is set use \"{?<index>} ... {?}\".\n"
        "For example to only print a second value if it is set use \"{?2} Second value: {2}{?}\".\n"
        "If a \"{?}\" is found without an opener, it is printed as is.\n"
        "\n"
        "To only print something if a variable is not set, do the same as with if, just replace every '?' with a '!'.\n"
        "For example to print a fallback for a second value if it is not set use \"{?2}{2}{?}{/2}Second value fallback{/}\".\n"
        "\n"
        "There is a special variable set if an error occured during detection, you can access it with \"{e}\", \"{error}\" or \"{0}\".\n"
        "You can use ifs and not ifs with it like with an index, for example use \"{?e}some text{?}\" to print an text if an error occurred.\n"
        "\n"
        "To stop formating at any point in the format string, use \"{-}\".\n"
        "For example to print an error instead of the normal output if it occured, prefix the format string with \"{?e}{e}{-}{?}...\".\n"
        "\n"
        "To print something with color, start a placeholder with a '#' and then the linux terminal color encoding.\n"
        "\"\\033[\" at the start and a 'm' at the end is automatically added, so don't do that.\n"
        "A \"{#}\" is equivalent to a \"{#0}\" and resets everything to normal.\n"
        "For example to print something pink and underline use \"{#4;35}...{#}\".\n"
        "If not in a format string, fastfetch wraps errors with \"{#1;31}error{#}\", so you might want to do that to if you show errors.\n"
        "Information about what the numbers mean can be found here: https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_(Select_Graphic_Rendition)_parameters.\n"
        "Which escape codes are supported and how they look like is defined by your terminal.\n"
        "\n"
        "If an format string evaluates to an empty value, the whole line in the output will be discarded.\n"
        "You can therefore use --host-format \" \" to disable host output.\n"
        "Note that --host-format \"\" would evaluate as not set, and therefore use the built in host format\n"
        "This can be used to print nothing if an error occured: prefix the format string with \"{?e}{-}{?}...\".\n"
        "\n"
        "Format string is also the way to go to set a fixed value, just use one without placeholders.\n"
        "For example when running in headless mode you could use \"--resolution-format \"Preferred\"."
    );
}

static inline void printAvailableModules()
{
    puts(
        "Battery\n"
        "Break\n"
        "Colors\n"
        "CPU\n"
        "DE\n"
        "Disk\n"
        "Font\n"
        "GPU\n"
        "Host\n"
        "Icons\n"
        "Kernel\n"
        "Locale\n"
        "Memory\n"
        "OS\n"
        "Packages\n"
        "Resolution\n"
        "Seperator\n"
        "Shell\n"
        "Terminal\n"
        "TerminalFont\n"
        "Theme\n"
        "Title\n"
        "Uptime\n"
        "WM\n"
        "WMTheme\n"
        "\n"
        "+ Additional defined by --set"
    );
}

static void constructAndPrintCommandHelpFormat(const char* name, const char* def, uint32_t numArgs, ...)
{
    va_list argp;
    va_start(argp, numArgs);

    printf("--%s-format:\n", name);
    printf("Sets the format string for %s output.\n", name);
    puts("To see how a format string is constructed, take a look at \"fastfetch --help format\".");
    puts("Following values are passed:");

    for(uint32_t i = 1; i <= numArgs; i++)
        printf("        {%u}: %s\n", i, va_arg(argp, const char*));

    printf("The default is something like \"%s\".\n", def);

    va_end(argp);
}

static inline void printCommandHelp(const char* command)
{
    if(strcasecmp(command, "c") == 0 || strcasecmp(command, "color") == 0)
        printCommandHelpColor();
    else if(strcasecmp(command, "format") == 0)
        printCommandHelpFormat();
    else if(strcasecmp(command, "os-format") == 0)
    {
        constructAndPrintCommandHelpFormat("os", "{3} {12}", 12,
            "System name, typically just Linux",
            "Name of the OS",
            "Pretty name of the OS",
            "ID of the OS",
            "ID like of the OS",
            "Variant of the OS",
            "Variant ID of the OS",
            "Version of the OS",
            "Version ID of the OS",
            "Version codename of the OS",
            "Build ID of the OS",
            "Architecture of the OS"
        );
    }
    else if(strcasecmp(command, "host-format") == 0)
    {
        constructAndPrintCommandHelpFormat("host", "{} {} {}", 3,
            "Host family",
            "Host name",
            "Host version"
        );
    }
    else if(strcasecmp(command, "kernel-format") == 0)
    {
        constructAndPrintCommandHelpFormat("kernel", "{2}", 3,
            "Kernel sysname",
            "Kernel release",
            "Kernel version"
        );
    }
    else if(strcasecmp(command, "uptime-format") == 0)
    {
        constructAndPrintCommandHelpFormat("uptime", "{} days {} hours {} mins", 4,
            "Days",
            "Hours",
            "Minutes",
            "Seconds"
        );
    }
    else if(strcasecmp(command, "packages-format") == 0)
    {
        constructAndPrintCommandHelpFormat("packages", "{2} (pacman), {3} (flatpak)", 3,
            "Number of all packages",
            "Number of pacman packages",
            "Number of flatpak packages"
        );
    }
    else if(strcasecmp(command, "shell-format") == 0)
    {
        constructAndPrintCommandHelpFormat("shell", "{2} {4}", 4,
            "Shell path (without name)",
            "Shell name",
            "Shell version raw",
            "Shell version pretty"
        );
    }
    else if(strcasecmp(command, "resolution-format") == 0)
    {
        constructAndPrintCommandHelpFormat("resolution", "{}x{} @ {}Hz", 3,
            "Screen width",
            "Screen height",
            "Screen refresh rate"
        );
    }
    else if(strcasecmp(command, "de-format") == 0)
    {
        constructAndPrintCommandHelpFormat("de", "{} {} ({})", 3,
            "DE name",
            "DE version",
            "Name of display server"
        );
    }
    else if(strcasecmp(command, "wm-format") == 0)
    {
        constructAndPrintCommandHelpFormat("wm", "{2}", 2,
            "WM process name",
            "WM pretty name"
        );
    }
    else if(strcasecmp(command, "wm-theme-format") == 0)
    {
        constructAndPrintCommandHelpFormat("wm-theme", "{}", 1,
            "WM theme name"
        );
    }
    else if(strcasecmp(command, "theme-format") == 0)
    {
        constructAndPrintCommandHelpFormat("theme", "{} ({3}) [Plasma], {7}", 7,
            "Plasma theme",
            "Plasma color scheme",
            "Plasma color scheme pretty",
            "GTK2 theme",
            "GTK3 theme",
            "GTK4 theme",
            "Combined GTK themes"
        );
    }
    else if(strcasecmp(command, "icons-format") == 0)
    {
        constructAndPrintCommandHelpFormat("icons", "{} [Plasma], {5}", 5,
            "Plasma icons",
            "GTK2 icons",
            "GTK3 icons",
            "GTK4 icons",
            "Combined GTK icons"
        );
    }
    else if(strcasecmp(command, "font-format") == 0)
    {
        constructAndPrintCommandHelpFormat("font", "{4}, {17}", 17,
            "Plasma as in resource file",
            "Plasma name",
            "Plasma size",
            "Plasma name and size pretty together",
            "GTK2 as in resource file",
            "GTK2 name",
            "GTK2 size",
            "GTK2 name and size pretty together",
            "GTK3 as in resource file",
            "GTK3 name",
            "GTK3 size",
            "GTK3 name and size pretty together",
            "GTK4 as in resource file",
            "GTK4 name",
            "GTK4 size",
            "GTK4 name and size pretty together",
            "GTK 2/3/4 name and size pretty together"
        );
    }
    else if(strcasecmp(command, "terminal-format") == 0)
    {
        constructAndPrintCommandHelpFormat("terminal", "{3}", 3,
            "Terminal executable name",
            "Terminal process name",
            "Terminal name"
        );
    }
    else if(strcasecmp(command, "terminal-font-format") == 0)
    {
        constructAndPrintCommandHelpFormat("terminal-font", "{4}", 4,
            "Terminal font as in resource file",
            "Terminal font name",
            "Termianl font size",
            "Terminal font name and size pretty together"
        );
    }
    else if(strcasecmp(command, "cpu-format") == 0)
    {
        constructAndPrintCommandHelpFormat("cpu", "{2} ({7}) @ {13}GHz", 13,
            "CPU name",
            "Prettified CPU name",
            "CPU Vendor name (Vendor ID)",
            "CPU logical core count online",
            "CPU logical core count configured",
            "CPU physical core count",
            "Always set core count",
            "frequency bios limit",
            "frequency scaling max",
            "frequency scaling min",
            "frequency info max",
            "frequency info min",
            "Always set frequeny"
        );
    }
    else if(strcasecmp(command, "gpu-format") == 0)
    {
        constructAndPrintCommandHelpFormat("gpu", "{2} {3}", 3,
            "GPU vendor",
            "GPU vendor pretty"
            "GPU name"
        );
    }
    else if(strcasecmp(command, "memory-format") == 0)
    {
        constructAndPrintCommandHelpFormat("memory", "{}MiB / {}MiB ({}%)", 3,
            "Used memory",
            "Total memory",
            "Used memory percentage"
        );
    }
    else if(strcasecmp(command, "disk-format") == 0)
    {
        constructAndPrintCommandHelpFormat("disk", "{}GB / {}GB ({4}%)", 4,
            "Used disk space",
            "Total disk space",
            "Number of files",
            "Used disk space percentage"
        );
    }
    else if(strcasecmp(command, "battery-format") == 0)
    {
        constructAndPrintCommandHelpFormat("battery", "{} {} ({}) [{}%; {}]", 5,
            "Battery manufactor",
            "Battery model",
            "Battery technology",
            "Battery capacity",
            "Battery status"
        );
    }
    else if(strcasecmp(command, "locale-format") == 0)
    {
        constructAndPrintCommandHelpFormat("locale", "{}", 1,
            "Locale code"
        );
    }
    else
        fprintf(stderr, "No specific help for command %s provided\n", command);
}

static inline bool optionParseBoolean(const char* str)
{
    if(str == NULL)
        return true;

    return (
        strcasecmp(str, "true") == 0 ||
        strcasecmp(str, "yes")  == 0 ||
        strcasecmp(str, "on")   == 0 ||
        strcasecmp(str, "1")    == 0
    );
}

static inline void optionParseString(const char* key, const char* value, FFstrbuf* buffer)
{
    if(value == NULL)
    {
        fprintf(stderr, "Error: usage: %s <str>\n", key);
        exit(477);
    }
    ffStrbufEnsureCapacity(buffer, 64); //This is not needed as ffStrbufSetS will resize capacity if needed, but giving an higher start should improve performance
    ffStrbufSetS(buffer, value);
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
    else if(strcasecmp(key, "--print-default-structure") == 0)
    {
        puts(FASTFETCH_DEFAULT_STRUCTURE);
        exit(0);
    }
    else if(strcasecmp(key, "--print-available-modules") == 0)
    {
        printAvailableModules();
        exit(0);
    }
    else if(strcasecmp(key, "--spacing") == 0)
    {
        if(value == NULL)
        {
            fprintf(stderr, "Error: usage: %s <width>\n", key);
            exit(404);
        }
        if(sscanf(value, "%hd", &instance->config.logo_spacing) != 1)
        {
            fprintf(stderr, "Error: couldn't parse %s to uint16_t\n", value);
            exit(405);
        }
    }
    else if(strcasecmp(key, "-x") == 0 || strcasecmp(key, "--offsetx") == 0)
    {
        if(value == NULL)
        {
            fprintf(stderr, "Error: usage: %s <offset>\n", key);
            exit(408);
        }
        if(sscanf(value, "%hi", &instance->config.offsetx) != 1)
        {
            fprintf(stderr, "Error: couldn't parse %s to int16_t\n", value);
            exit(409);
        }
    }
    else if(strcasecmp(key, "--set") == 0)
    {
        if(value == NULL)
        {
            fprintf(stderr, "Error: usage: %s <key=value>\n", key);
            exit(411);
        }

        char* seperator = strchr(value, '=');

        if(seperator == NULL)
        {
            fprintf(stderr, "Error: usage: %s <key=value>, '=' missing\n", key);
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
    else if(strcasecmp(key, "--nocache") == 0)
    {
        instance->config.recache = optionParseBoolean(value);
        instance->config.cacheSave = false;
    }
    else if(strcasecmp(key, "--show-errors") == 0)
        instance->config.showErrors = optionParseBoolean(value);
    else if(strcasecmp(key, "--color-logo") == 0)
        instance->config.colorLogo = optionParseBoolean(value);
    else if(strcasecmp(key, "--print-remaining-logo") == 0)
        instance->config.printRemainingLogo = optionParseBoolean(value);
    else if(strcasecmp(key, "--multithreading") == 0)
        data->multithreading = optionParseBoolean(value);
    else if(strcasecmp(key, "--structure") == 0)
        optionParseString(key, value, &data->structure);
    else if(strcasecmp(key, "-l") == 0 || strcasecmp(key, "--logo") == 0)
        optionParseString(key, value, &data->logoName);
    else if(strcasecmp(key, "-s") == 0 || strcasecmp(key, "--seperator") == 0)
        optionParseString(key, value, &instance->config.seperator);
    else if(strcasecmp(key, "-c") == 0 || strcasecmp(key, "--color") == 0)
        optionParseString(key, value, &instance->config.color);
    else if(strcasecmp(key, "--os-format") == 0)
        optionParseString(key, value, &instance->config.osFormat);
    else if(strcasecmp(key, "--os-key") == 0)
        optionParseString(key, value, &instance->config.osKey);
    else if(strcasecmp(key, "--host-format") == 0)
        optionParseString(key, value, &instance->config.hostFormat);
    else if(strcasecmp(key, "--host-key") == 0)
        optionParseString(key, value, &instance->config.hostKey);
    else if(strcasecmp(key, "--kernel-format") == 0)
        optionParseString(key, value, &instance->config.kernelFormat);
    else if(strcasecmp(key, "--kernel-key") == 0)
        optionParseString(key, value, &instance->config.kernelKey);
    else if(strcasecmp(key, "--uptime-format") == 0)
        optionParseString(key, value, &instance->config.uptimeFormat);
    else if(strcasecmp(key, "--uptime-key") == 0)
        optionParseString(key, value, &instance->config.uptimeKey);
    else if(strcasecmp(key, "--packages-format") == 0)
        optionParseString(key, value, &instance->config.packagesFormat);
    else if(strcasecmp(key, "--packages-key") == 0)
        optionParseString(key, value, &instance->config.packagesKey);
    else if(strcasecmp(key, "--shell-format") == 0)
        optionParseString(key, value, &instance->config.shellFormat);
    else if(strcasecmp(key, "--shell-key") == 0)
        optionParseString(key, value, &instance->config.shellKey);
    else if(strcasecmp(key, "--resolution-format") == 0)
        optionParseString(key, value, &instance->config.resolutionFormat);
    else if(strcasecmp(key, "--resolution-key") == 0)
        optionParseString(key, value, &instance->config.resolutionKey);
    else if(strcasecmp(key, "--de-format") == 0)
        optionParseString(key, value, &instance->config.deFormat);
    else if(strcasecmp(key, "--de-key") == 0)
        optionParseString(key, value, &instance->config.deKey);
    else if(strcasecmp(key, "--wm-format") == 0)
        optionParseString(key, value, &instance->config.wmFormat);
    else if(strcasecmp(key, "--wm-key") == 0)
        optionParseString(key, value, &instance->config.wmKey);
    else if(strcasecmp(key, "--wm-theme-format") == 0)
        optionParseString(key, value, &instance->config.wmThemeFormat);
    else if(strcasecmp(key, "--wm-theme-key") == 0)
        optionParseString(key, value, &instance->config.wmThemeKey);
    else if(strcasecmp(key, "--theme-format") == 0)
        optionParseString(key, value, &instance->config.themeFormat);
    else if(strcasecmp(key, "--theme-key") == 0)
        optionParseString(key, value, &instance->config.themeKey);
    else if(strcasecmp(key, "--icons-format") == 0)
        optionParseString(key, value, &instance->config.iconsFormat);
    else if(strcasecmp(key, "--icons-key") == 0)
        optionParseString(key, value, &instance->config.iconsKey);
    else if(strcasecmp(key, "--font-format") == 0)
        optionParseString(key, value, &instance->config.fontFormat);
    else if(strcasecmp(key, "--font-key") == 0)
        optionParseString(key, value, &instance->config.fontKey);
    else if(strcasecmp(key, "--terminal-format") == 0)
        optionParseString(key, value, &instance->config.terminalFormat);
    else if(strcasecmp(key, "--terminal-key") == 0)
        optionParseString(key, value, &instance->config.terminalKey);
    else if(strcasecmp(key, "--terminal-font-format") == 0)
        optionParseString(key, value, &instance->config.termFontFormat);
    else if(strcasecmp(key, "--terminal-font-key") == 0)
        optionParseString(key, value, &instance->config.termFontKey);
    else if(strcasecmp(key, "--cpu-format") == 0)
        optionParseString(key, value, &instance->config.cpuFormat);
    else if(strcasecmp(key, "--cpu-key") == 0)
        optionParseString(key, value, &instance->config.cpuKey);
    else if(strcasecmp(key, "--gpu-format") == 0)
        optionParseString(key, value, &instance->config.gpuFormat);
    else if(strcasecmp(key, "--gpu-key") == 0)
        optionParseString(key, value, &instance->config.gpuKey);
    else if(strcasecmp(key, "--memory-format") == 0)
        optionParseString(key, value, &instance->config.memoryFormat);
    else if(strcasecmp(key, "--memory-key") == 0)
        optionParseString(key, value, &instance->config.memoryKey);
    else if(strcasecmp(key, "--disk-format") == 0)
        optionParseString(key, value, &instance->config.diskFormat);
    else if(strcasecmp(key, "--disk-key") == 0)
        optionParseString(key, value, &instance->config.diskKey);
    else if(strcasecmp(key, "--battery-format") == 0)
        optionParseString(key, value, &instance->config.batteryFormat);
    else if(strcasecmp(key, "--battery-key") == 0)
        optionParseString(key, value, &instance->config.batteryKey);
    else if(strcasecmp(key, "--locale-format") == 0)
        optionParseString(key, value, &instance->config.localeFormat);
    else if(strcasecmp(key, "--locale-key") == 0)
        optionParseString(key, value, &instance->config.localeKey);
    else if(strcasecmp(key, "--lib-PCI") == 0)
        optionParseString(key, value, &instance->config.libPCI);
    else if(strcasecmp(key, "--lib-X11") == 0)
        optionParseString(key, value, &instance->config.libX11);
    else if(strcasecmp(key, "--lib-Xrandr") == 0)
        optionParseString(key, value, &instance->config.libXrandr);
    else if(strcasecmp(key, "--lib-DConf") == 0)
        optionParseString(key, value, &instance->config.libDConf);
    else if(strcasecmp(key, "--lib-wayland") == 0)
        optionParseString(key, value, &instance->config.libWayland);
    else if(strcasecmp(key, "--disk-folders") == 0)
        optionParseString(key, value, &instance->config.diskFolders);
    else if(strcasecmp(key, "--battery-dir") == 0)
        optionParseString(key, value, &instance->config.batteryDir);
    else
    {
        fprintf(stderr, "Error: unknown option: %s\n", key);
        exit(400);
    }
}

static void parseConfigFile(FFinstance* instance, FFdata* data)
{
    FFstrbuf filename;
    ffStrbufInitA(&filename, 64);

    const char* xdgConfig = getenv("XDG_CONFIG_HOME");
    if(xdgConfig == NULL)
    {
        ffStrbufAppendS(&filename, instance->state.passwd->pw_dir);
        ffStrbufAppendS(&filename, "/.config");
    }
    else
    {
        ffStrbufAppendS(&filename, xdgConfig);
    }
    mkdir(filename.chars, S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH); //I hope everybody has a config folder but whow knews

    ffStrbufAppendS(&filename, "/fastfetch/");
    mkdir(filename.chars, S_IRWXU | S_IRGRP | S_IROTH);

    ffStrbufAppendS(&filename, "config.conf");

    if(access(filename.chars, F_OK) != 0)
    {
        FILE* file = fopen(filename.chars, "w");
        fputs(FASTFETCH_DEFAULT_CONFIG, file);
        fclose(file);
        return;
    }

    FILE* file = fopen(filename.chars, "r");

    char* lineStart = NULL;
    size_t len = 0;
    ssize_t read;

    FFstrbuf line;
    ffStrbufInitA(&line, 128); //The default structure line needs this size

    while ((read = getline(&lineStart, &len, file)) != -1)
    {
        ffStrbufSetS(&line, lineStart);
        ffStrbufTrimRight(&line, '\n');
        ffStrbufTrim(&line, ' ');

        if(line.length == 0 || line.chars[0] == '#')
            continue;

        uint32_t firstSpace = ffStrbufFirstIndexC(&line, ' ');

        if(firstSpace >= line.length)
        {
            parseOption(instance, data, line.chars, NULL);
            ffStrbufDestroy(&line);
            continue;
        }

        //Seperate key and value by simply replacing the first space with a \0
        char* valueStart = &line.chars[firstSpace];
        *valueStart = '\0';
        ++valueStart;

        //Trim whitespace at beginn of value
        while(*valueStart == ' ')
            ++valueStart;

        //If we want whitespace in values, we need to quote it. This is done to keep consistency with shell.
        if(*valueStart == '"')
        {
            char* last = line.chars + line.length - 1;
            if(*last == '"')
            {
                ++valueStart;
                *last = '\0';
                --line.length;
            }
        }

        parseOption(instance, data, line.chars, valueStart);
    }

    ffStrbufDestroy(&line);

    if(lineStart != NULL)
        free(lineStart);

    fclose(file);

    ffStrbufDestroy(&filename);
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
    if(data->logoName.length == 0)
        ffLoadLogo(instance);
    else
        ffLoadLogoSet(&instance->config, data->logoName.chars);

    //This must be done after loading the logo
    if(instance->config.color.length == 0)
        ffStrbufSetS(&instance->config.color, instance->config.logo.color);
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
    else if(strcasecmp(line, "wmtheme") == 0)
        ffPrintWMTheme(instance);
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
        ffPrintError(instance, line, 0, NULL, NULL, 0, "<no implementation provided>");
}

static void run(FFinstance* instance, FFdata* data)
{
    if(data->multithreading)
        ffStartCalculationThreads(instance);

    if(data->structure.length == 0)
        ffStrbufSetS(&data->structure, FASTFETCH_DEFAULT_STRUCTURE);

    uint32_t lastIndex = 0;
    while (lastIndex < data->structure.length)
    {
        uint32_t colonIndex = ffStrbufFirstIndexAfterC(&data->structure, lastIndex, ':');
        data->structure.chars[colonIndex] = '\0';

        parseStructureCommand(instance, data, data->structure.chars + lastIndex);

        lastIndex = colonIndex + 1;
    }

    ffFinish(instance);
}

static void initData(FFdata* data)
{
    ffValuestoreInit(&data->valuestore);
    ffStrbufInitA(&data->structure, 256);
    ffStrbufInit(&data->logoName);
    data->multithreading = true;
}

int main(int argc, const char** argv)
{
    FFinstance instance;
    ffInitInstance(&instance);

    FFdata data;
    initData(&data);

    parseConfigFile(&instance, &data);
    parseArguments(&instance, &data, argc, argv);
    applyData(&instance, &data); //Here we do things that need to be done after parsing all options

    run(&instance, &data);
}
