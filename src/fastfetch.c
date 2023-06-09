#include "fastfetch.h"
#include "common/printing.h"
#include "common/parsing.h"
#include "common/io/io.h"
#include "common/time.h"
#include "util/stringUtils.h"
#include "logo/logo.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

#ifdef WIN32
    #include "util/windows/getline.h"
#endif

#pragma GCC diagnostic ignored "-Wsign-conversion"

#include "common/jsonconfig.h"
#include "modules/modules.h"

typedef struct CustomValue
{
    bool printKey;
    FFstrbuf value;
} CustomValue;

// Things only needed by fastfetch
typedef struct FFdata
{
    FFstrbuf structure;
    bool loadUserConfig;
} FFdata;

static void constructAndPrintCommandHelpFormat(const char* name, const char* def, uint32_t numArgs, ...)
{
    va_list argp;
    va_start(argp, numArgs);

    printf("--%s-format:\n", name);
    printf("Sets the format string for %s output.\n", name);
    puts("To see how a format string is constructed, take a look at \"fastfetch --help format\".");
    puts("The following values are passed:");

    for(uint32_t i = 1; i <= numArgs; i++)
        printf("        {%u}: %s\n", i, va_arg(argp, const char*));

    printf("The default is something similar to \"%s\".\n", def);

    va_end(argp);
}

static inline void printCommandHelp(const char* command)
{
    if(command == NULL)
        puts(FASTFETCH_DATATEXT_HELP);
    else if(strcasecmp(command, "c") == 0 || strcasecmp(command, "color") == 0)
        puts(FASTFETCH_DATATEXT_HELP_COLOR);
    else if(strcasecmp(command, "format") == 0)
        puts(FASTFETCH_DATATEXT_HELP_FORMAT);
    else if(strcasecmp(command, "load-config") == 0 || strcasecmp(command, "loadconfig") == 0 || strcasecmp(command, "config") == 0)
        puts(FASTFETCH_DATATEXT_HELP_CONFIG);
    else if(strcasecmp(command, "os-format") == 0)
    {
        constructAndPrintCommandHelpFormat("os", "{3} {12}", 12,
            "System name (typically just Linux)",
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
        constructAndPrintCommandHelpFormat("host", "{2} {3}", 8,
            "product family",
            "product name",
            "product version",
            "product sku",
            "chassis type",
            "chassis vendor",
            "chassis version",
            "sys vendor"
        );
    }
    else if(strcasecmp(command, "bios-format") == 0)
    {
        constructAndPrintCommandHelpFormat("bios", "{2} {3}", 4,
            "bios date",
            "bios release",
            "bios vendor",
            "bios version"
        );
    }
    else if(strcasecmp(command, "board-format") == 0)
    {
        constructAndPrintCommandHelpFormat("board", "{2} {3}", 3,
            "board name",
            "board vendor",
            "board version"
        );
    }
    else if(strcasecmp(command, "chassis-format") == 0)
    {
        constructAndPrintCommandHelpFormat("chassis", "{2} {3}", 4,
            "chassis type",
            "chassis vendor",
            "chassis version"
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
    else if(strcasecmp(command, "processes-format") == 0)
    {
        constructAndPrintCommandHelpFormat("processes", "{}", 1,
            "Count"
        );
    }
    else if(strcasecmp(command, "packages-format") == 0)
    {
        constructAndPrintCommandHelpFormat("packages", "{2} (pacman){?3}[{3}]{?}, {4} (dpkg), {5} (rpm), {6} (emerge), {7} (eopkg), {8} (xbps), {9} (nix-system), {10} (nix-user), {11} (nix-default), {12} (apk), {13} (pkg), {14} (flatpak-system), {15} (flatpack-user), {16} (snap), {17} (brew), {18} (brew-cask), {19} (port), {20} (scoop), {21} (choco)", 21,
            "Number of all packages",
            "Number of pacman packages",
            "Pacman branch on manjaro",
            "Number of dpkg packages",
            "Number of rpm packages",
            "Number of emerge packages",
            "Number of eopkg packages",
            "Number of xbps packages",
            "Number of nix-system packages",
            "Number of nix-user packages",
            "Number of nix-default packages",
            "Number of apk packages",
            "Number of pkg packages",
            "Number of flatpak-system packages",
            "Number of flatpak-user packages",
            "Number of snap packages",
            "Number of brew packages",
            "Number of brew-cask packages",
            "Number of macports packages",
            "Number of scoop packages",
            "Number of choco packages"
        );
    }
    else if(strcasecmp(command, "shell-format") == 0)
    {
        constructAndPrintCommandHelpFormat("shell", "{3} {4}", 7,
            "Shell process name",
            "Shell path with exe name",
            "Shell exe name",
            "Shell version",
            "User shell path with exe name",
            "User shell exe name",
            "User shell version"
        );
    }
    else if(strcasecmp(command, "display-format") == 0)
    {
        constructAndPrintCommandHelpFormat("display", "{}x{} @ {}Hz", 5,
            "Screen width",
            "Screen height",
            "Screen refresh rate",
            "Screen scaled width",
            "Screen scaled height"
        );
    }
    else if(strcasecmp(command, "de-format") == 0)
    {
        constructAndPrintCommandHelpFormat("de", "{2} {3}", 3,
            "DE process name",
            "DE pretty name",
            "DE version"
        );
    }
    else if(strcasecmp(command, "wm-format") == 0)
    {
        constructAndPrintCommandHelpFormat("wm", "{2} ({3})", 3,
            "WM process name",
            "WM pretty name",
            "WM protocol name"
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
        constructAndPrintCommandHelpFormat("theme", "{}", 1,
            "Combined themes"
        );
    }
    else if(strcasecmp(command, "icons-format") == 0)
    {
        constructAndPrintCommandHelpFormat("icons", "{}", 1,
            "Combined icons"
        );
    }
    else if(strcasecmp(command, "wallpaper-format") == 0)
    {
        constructAndPrintCommandHelpFormat("wallpaper", "{}", 1,
            "Wallpaper image file"
        );
    }
    else if(strcasecmp(command, "font-format") == 0)
    {
        constructAndPrintCommandHelpFormat("font", "{} [QT], {} [GTK2], {} [GTK3], {} [GTK4]", 4,
            "Font 1",
            "Font 2",
            "Font 3",
            "Font 4"
        );
    }
    else if(strcasecmp(command, "cursor-format") == 0)
    {
        constructAndPrintCommandHelpFormat("cursor", "{} ({}pt)", 2,
            "Cursor theme",
            "Cursor size"
        );
    }
    else if(strcasecmp(command, "terminal-format") == 0)
    {
        constructAndPrintCommandHelpFormat("terminal", "{3}", 10,
            "Terminal process name",
            "Terminal path with exe name",
            "Terminal exe name",
            "Shell process name",
            "Shell path with exe name",
            "Shell exe name",
            "Shell version",
            "User shell path with exe name",
            "User shell exe name",
            "User shell version"
        );
    }
    else if(strcasecmp(command, "terminal-font-format") == 0)
    {
        constructAndPrintCommandHelpFormat("terminal-font", "{}", 4,
            "Terminal font",
            "Terminal font name",
            "Termianl font size",
            "Terminal font styles"
        );
    }
    else if(strcasecmp(command, "cpu-format") == 0)
    {
        constructAndPrintCommandHelpFormat("cpu", "{1} ({5}) @ {7}GHz", 8,
            "Name",
            "Vendor",
            "Physical core count",
            "Logical core count",
            "Online core count",
            "Min frequency",
            "Max frequency",
            "Temperature"
        );
    }
    else if(strcasecmp(command, "cpu-usage-format") == 0)
    {
        constructAndPrintCommandHelpFormat("cpu-usage", "{0}%", 1,
            "CPU usage without percent mark"
        );
    }
    else if(strcasecmp(command, "gpu-format") == 0)
    {
        constructAndPrintCommandHelpFormat("gpu", "{} {}", 6,
            "GPU vendor",
            "GPU name",
            "GPU driver",
            "GPU temperature",
            "GPU core count",
            "GPU type"
        );
    }
    else if(strcasecmp(command, "memory-format") == 0)
    {
        constructAndPrintCommandHelpFormat("memory", "{} / {} ({}%)", 3,
            "Used size",
            "Total size",
            "Percentage used"
        );
    }
    else if(strcasecmp(command, "swap-format") == 0)
    {
        constructAndPrintCommandHelpFormat("swap", "{} / {} ({}%)", 3,
            "Used size",
            "Total size",
            "Percentage used"
        );
    }
    else if(strcasecmp(command, "disk-format") == 0)
    {
        constructAndPrintCommandHelpFormat("disk", "{1} / {2} ({3}%)", 9,
            "Size used",
            "Size total",
            "Size percentage",
            "Files used",
            "Files total",
            "Files percentage",
            "True if removable volume",
            "True if hidden volume",
            "Filesystem"
        );
    }
    else if(strcasecmp(command, "battery-format") == 0)
    {
        constructAndPrintCommandHelpFormat("battery", "{}%, {}", 5,
            "Battery manufactor",
            "Battery model",
            "Battery technology",
            "Battery capacity",
            "Battery status"
        );
    }
    else if(strcasecmp(command, "poweradapter-format") == 0)
    {
        constructAndPrintCommandHelpFormat("poweradapter", "{}%, {}", 5,
            "PowerAdapter watts",
            "PowerAdapter name",
            "PowerAdapter manufacturer",
            "PowerAdapter model",
            "PowerAdapter description"
        );
    }
    else if(strcasecmp(command, "locale-format") == 0)
    {
        constructAndPrintCommandHelpFormat("locale", "{}", 1,
            "Locale code"
        );
    }
    else if(strcasecmp(command, "local-ip-format") == 0)
    {
        constructAndPrintCommandHelpFormat("local-ip", "{}", 1,
            "Local IP address"
        );
    }
    else if(strcasecmp(command, "public-ip-format") == 0)
    {
        constructAndPrintCommandHelpFormat("public-ip", "{}", 1,
            "Public IP address"
        );
    }
    else if(strcasecmp(command, "wifi-format") == 0)
    {
        constructAndPrintCommandHelpFormat("wifi", "{4} - {6}", 3,
            "Interface description",
            "Interface status",
            "Connection status",
            "Connection SSID",
            "Connection mac address",
            "Connection protocol",
            "Connection signal quality (percentage)",
            "Connection RX rate",
            "Connection TX rate",
            "Connection Security algorithm"
        );
    }
    else if(strcasecmp(command, "player-format") == 0)
    {
        constructAndPrintCommandHelpFormat("player", "{}", 4,
            "Pretty player name",
            "Player name",
            "Player Identifier",
            "URL name"
        );
    }
    else if(strcasecmp(command, "media-format") == 0)
    {
        constructAndPrintCommandHelpFormat("media", "{3} - {1}", 4,
            "Pretty media name",
            "Media name",
            "Artist name",
            "Album name"
        );
    }
    else if(strcasecmp(command, "datetime-format") == 0 || strcasecmp(command, "date-format") == 0 || strcasecmp(command, "time-format") == 0)
    {
        constructAndPrintCommandHelpFormat("[date][time]", "{1}-{4}-{11} {14}:{18}:{20}", 20,
            "year",
            "last two digits of year",
            "month",
            "month with leading zero",
            "month name",
            "month name short",
            "week number on year",
            "weekday",
            "weekday short",
            "day in year",
            "day in month",
            "day in Week",
            "hour",
            "hour with leading zero",
            "hour 12h format",
            "hour 12h format with leading zero",
            "minute",
            "minute with leading zero",
            "second",
            "second with leading zero"
        );
    }
    else if(strcasecmp(command, "vulkan-format") == 0)
    {
        constructAndPrintCommandHelpFormat("vulkan", "{} (driver), {} (api version)", 3,
            "Driver name",
            "API version",
            "Conformance version"
        );
    }
    else if(strcasecmp(command, "opengl-format") == 0)
    {
        constructAndPrintCommandHelpFormat("opengl", "{}", 3,
            "version",
            "renderer",
            "vendor",
            "shading language version"
        );
    }
    else if(strcasecmp(command, "opencl-format") == 0)
    {
        constructAndPrintCommandHelpFormat("opencl", "{}", 3,
            "version",
            "device",
            "vendor"
        );
    }
    else if(strcasecmp(command, "bluetooth-format") == 0)
    {
        constructAndPrintCommandHelpFormat("bluetooth", "{1} (4%)", 4,
            "Name",
            "Address",
            "Type",
            "Battery percentage"
        );
    }
    else if(strcasecmp(command, "sound-format") == 0)
    {
        constructAndPrintCommandHelpFormat("sound", "{2} (3%)", 4,
            "Main",
            "Name",
            "Volume",
            "Identifier"
        );
    }
    else if(strcasecmp(command, "gamepad-format") == 0)
    {
        constructAndPrintCommandHelpFormat("gamepad", "{1}", 1,
            "Name",
            "Identifier"
        );
    }
    else
        fprintf(stderr, "No specific help for command %s provided\n", command);
}

static void listAvailablePresets(FFinstance* instance)
{
    FF_LIST_FOR_EACH(FFstrbuf, path, instance->state.platform.dataDirs)
    {
        ffStrbufAppendS(path, "fastfetch/presets/");
        ffListFilesRecursively(path->chars);
    }
}

static void listAvailableLogos(FFinstance* instance)
{
    FF_LIST_FOR_EACH(FFstrbuf, path, instance->state.platform.dataDirs)
    {
        ffStrbufAppendS(path, "fastfetch/logos/");
        ffListFilesRecursively(path->chars);
    }
}

static void listConfigPaths(FFinstance* instance)
{
    FF_LIST_FOR_EACH(FFstrbuf, folder, instance->state.platform.configDirs)
    {
        ffStrbufAppendS(folder, "fastfetch/config.conf");
        printf("%s%s\n", folder->chars, ffPathExists(folder->chars, FF_PATHTYPE_FILE) ? " (*)" : "");
    }
}

static void listDataPaths(FFinstance* instance)
{
    FF_LIST_FOR_EACH(FFstrbuf, folder, instance->state.platform.dataDirs)
    {
        ffStrbufAppendS(folder, "fastfetch/");
        puts(folder->chars);
    }
}

static void parseOption(FFinstance* instance, FFdata* data, const char* key, const char* value);

static bool parseConfigFile(FFinstance* instance, FFdata* data, const char* path)
{
    FILE* file = fopen(path, "r");
    if(file == NULL)
        return false;

    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1)
    {
        char* lineStart = line;
        char* lineEnd = line + read - 1;

        //Trim line left
        while(isspace(*lineStart))
            ++lineStart;

        //Continue if line is empty or a comment
        if(*lineStart == '\0' || *lineStart == '#')
            continue;

        //Trim line right
        while(lineEnd > lineStart && isspace(*lineEnd))
            --lineEnd;
        *(lineEnd + 1) = '\0';

        char* valueStart = strchr(lineStart, ' ');

        //If the line has no white space, it is only a key
        if(valueStart == NULL)
        {
            parseOption(instance, data, lineStart, NULL);
            continue;
        }

        //separate the key from the value
        *valueStart = '\0';
        ++valueStart;

        //Trim space of value left
        while(isspace(*valueStart))
            ++valueStart;

        //If we want whitespace in values, we need to quote it. This is done to keep consistency with shell.
        if((*valueStart == '"' || *valueStart == '\'') && *valueStart == *lineEnd && lineEnd > valueStart)
        {
            ++valueStart;
            *lineEnd = '\0';
            --lineEnd;
        }

        parseOption(instance, data, lineStart, valueStart);
    }

    if(line != NULL)
        free(line);

    fclose(file);
    return true;
}

static void generateConfigFile(FFinstance* instance, bool force)
{
    FFstrbuf* filename = (FFstrbuf*) ffListGet(&instance->state.platform.configDirs, 0);
    // Paths generated in `init.c/initConfigDirs` end with `/`
    ffStrbufAppendS(filename, "fastfetch/config.conf");

    if (!force && ffPathExists(filename->chars, FF_PATHTYPE_FILE))
    {
        fprintf(stderr, "Config file exists in `%s`, use `--gen-config-force` to overwrite\n", filename->chars);
        exit(1);
    }
    else
    {
        ffWriteFileData(filename->chars, sizeof(FASTFETCH_DATATEXT_CONFIG_USER), FASTFETCH_DATATEXT_CONFIG_USER);
        printf("A sample config file has been written in `%s`\n", filename->chars);
        exit(0);
    }
}

static void optionParseConfigFile(FFinstance* instance, FFdata* data, const char* key, const char* value)
{
    if(value == NULL)
    {
        fprintf(stderr, "Error: usage: %s <file>\n", key);
        exit(413);
    }

    //Try to load as an absolute path

    if(parseConfigFile(instance, data, value))
        return;

    //Try to load as a relative path

    FF_STRBUF_AUTO_DESTROY absolutePath = ffStrbufCreateA(128);

    FF_LIST_FOR_EACH(FFstrbuf, path, instance->state.platform.dataDirs)
    {
        //We need to copy it, because if a config file loads a config file, the value of path must be unchanged
        ffStrbufSet(&absolutePath, path);
        ffStrbufAppendS(&absolutePath, "fastfetch/presets/");
        ffStrbufAppendS(&absolutePath, value);

        bool success = parseConfigFile(instance, data, absolutePath.chars);

        if(success)
            return;
    }

    //File not found

    fprintf(stderr, "Error: couldn't find config: %s\n", value);
    exit(414);
}

static bool optionParseBoolean(const char* str)
{
    return (
        !ffStrSet(str) ||
        strcasecmp(str, "true") == 0 ||
        strcasecmp(str, "yes")  == 0 ||
        strcasecmp(str, "on")   == 0 ||
        strcasecmp(str, "1")    == 0
    );
}

static inline void optionCheckString(const char* key, const char* value, FFstrbuf* buffer)
{
    if(value == NULL)
    {
        fprintf(stderr, "Error: usage: %s <str>\n", key);
        exit(477);
    }
    ffStrbufEnsureFree(buffer, 63); //This is not needed, as ffStrbufSetS will resize capacity if needed, but giving a higher start should improve performance
}

static void optionParseString(const char* key, const char* value, FFstrbuf* buffer)
{
    optionCheckString(key, value, buffer);
    ffStrbufSetS(buffer, value);
}

static inline bool startsWith(const char* str, const char* compareTo)
{
    return strncasecmp(str, compareTo, strlen(compareTo)) == 0;
}

static void optionParseColor(const char* key, const char* value, FFstrbuf* buffer)
{
    optionCheckString(key, value, buffer);

    while(*value != '\0')
    {
        #define FF_APPEND_COLOR_CODE_COND(prefix, code) \
            if(startsWith(value, #prefix)) { ffStrbufAppendS(buffer, code); value += strlen(#prefix); }

        FF_APPEND_COLOR_CODE_COND(reset_, "0;")
        else FF_APPEND_COLOR_CODE_COND(bright_, "1;")
        else FF_APPEND_COLOR_CODE_COND(black, "30")
        else FF_APPEND_COLOR_CODE_COND(red, "31")
        else FF_APPEND_COLOR_CODE_COND(green, "32")
        else FF_APPEND_COLOR_CODE_COND(yellow, "33")
        else FF_APPEND_COLOR_CODE_COND(blue, "34")
        else FF_APPEND_COLOR_CODE_COND(magenta, "35")
        else FF_APPEND_COLOR_CODE_COND(cyan, "36")
        else FF_APPEND_COLOR_CODE_COND(white, "37")
        else
        {
            ffStrbufAppendC(buffer, *value);
            ++value;
        }

        #undef FF_APPEND_COLOR_CODE_COND
    }
}

static uint32_t optionParseUInt32(const char* key, const char* value)
{
    if(value == NULL)
    {
        fprintf(stderr, "Error: usage: %s <num>\n", key);
        exit(480);
    }

    char* end;
    uint32_t num = (uint32_t) strtoul(value, &end, 10);
    if(*end != '\0')
    {
        fprintf(stderr, "Error: usage: %s <num>\n", key);
        exit(479);
    }

    return num;
}

static void optionParseEnum(const char* argumentKey, const char* requestedKey, void* result, ...)
{
    if(requestedKey == NULL)
    {
        fprintf(stderr, "Error: usage: %s <value>\n", argumentKey);
        exit(476);
    }

    va_list args;
    va_start(args, result);

    while(true)
    {
        const char* key = va_arg(args, const char*);
        if(key == NULL)
            break;

        int value = va_arg(args, int); //C standard guarantees that enumeration constants are presented as ints

        if(strcasecmp(requestedKey, key) == 0)
        {
            *(int*)result = value;
            va_end(args);
            return;
        }
    }

    va_end(args);

    fprintf(stderr, "Error: unknown %s value: %s\n", argumentKey, requestedKey);
    exit(478);
}

static bool optionParseModuleArgs(const char* argumentKey, const char* value, const char* moduleName, struct FFModuleArgs* result)
{
    const char* pkey = argumentKey;
    if(!(pkey[0] == '-' && pkey[1] == '-'))
        return false;

    pkey += 2;
    uint32_t moduleNameLen = (uint32_t)strlen(moduleName);
    if(strncasecmp(pkey, moduleName, moduleNameLen) != 0)
        return false;

    pkey += moduleNameLen;
    if(pkey[0] != '-')
        return false;

    pkey += 1;
    if(strcasecmp(pkey, "key") == 0)
    {
        optionParseString(argumentKey, value, &result->key);
        return true;
    }
    else if(strcasecmp(pkey, "format") == 0)
    {
        optionParseString(argumentKey, value, &result->outputFormat);
        return true;
    }
    else if(strcasecmp(pkey, "error") == 0)
    {
        optionParseString(argumentKey, value, &result->errorFormat);
        return true;
    }
    return false;
}

static void parseOption(FFinstance* instance, FFdata* data, const char* key, const char* value)
{
    ///////////////////////
    //Informative options//
    ///////////////////////

    if(strcasecmp(key, "-h") == 0 || strcasecmp(key, "--help") == 0)
    {
        printCommandHelp(value);
        exit(0);
    }
    else if(strcasecmp(key, "-v") == 0 || strcasecmp(key, "--version") == 0)
    {
        #ifndef NDEBUG
            #define FF_BUILD_TYPE "-debug"
        #else
            #define FF_BUILD_TYPE
        #endif

        #if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64)
            #define FF_ARCHITECTURE "x86_64"
        #elif defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(__i586__) || defined(__i586) || defined(__i686__) || defined(__i686)
            #define FF_ARCHITECTURE "i386"
        #elif defined(__aarch64__)
            #define FF_ARCHITECTURE "aarch64"
        #elif defined(__arm__)
            #define FF_ARCHITECTURE "arm"
        #elif defined(__mips__)
            #define FF_ARCHITECTURE "mips"
        #elif defined(__powerpc__) || defined(__powerpc)
            #define FF_ARCHITECTURE "powerpc"
        #elif defined(__riscv__) || defined(__riscv)
            #define FF_ARCHITECTURE "riscv"
        #elif defined(__s390x__)
            #define FF_ARCHITECTURE "s390x"
        #else
            #define FF_ARCHITECTURE "unknown"
        #endif

        puts("fastfetch " FASTFETCH_PROJECT_VERSION FASTFETCH_PROJECT_VERSION_TWEAK FF_BUILD_TYPE " (" FF_ARCHITECTURE ")");

        #undef FF_ARCHITECTURE
        #undef FF_BUILD_TYPE

        exit(0);
    }
    else if(strcasecmp(key, "--version-raw") == 0)
    {
        puts(FASTFETCH_PROJECT_VERSION);
        exit(0);
    }
    else if(startsWith(key, "--print"))
    {
        const char* subkey = key + strlen("--print");
        if(strcasecmp(subkey, "-config-system") == 0)
        {
            puts(FASTFETCH_DATATEXT_CONFIG_SYSTEM);
            exit(0);
        }
        else if(strcasecmp(subkey, "-config-user") == 0)
        {
            puts(FASTFETCH_DATATEXT_CONFIG_USER);
            exit(0);
        }
        else if(strcasecmp(subkey, "-structure") == 0)
        {
            puts(FASTFETCH_DATATEXT_STRUCTURE);
            exit(0);
        }
        else if(strcasecmp(subkey, "-logos") == 0)
        {
            ffLogoBuiltinPrint(instance);
            exit(0);
        }
        else
            goto error;
    }
    else if(startsWith(key, "--list"))
    {
        const char* subkey = key + strlen("--list");
        if(strcasecmp(subkey, "-modules") == 0)
        {
            puts(FASTFETCH_DATATEXT_MODULES);
            exit(0);
        }
        else if(strcasecmp(subkey, "-presets") == 0)
        {
            listAvailablePresets(instance);
            exit(0);
        }
        else if(strcasecmp(subkey, "-config-paths") == 0)
        {
            listConfigPaths(instance);
            exit(0);
        }
        else if(strcasecmp(subkey, "-data-paths") == 0)
        {
            listDataPaths(instance);
            exit(0);
        }
        else if(strcasecmp(subkey, "-features") == 0)
        {
            ffListFeatures();
            exit(0);
        }
        else if(strcasecmp(subkey, "-logos") == 0)
        {
            puts("Builtin logos:");
            ffLogoBuiltinList();
            puts("\nCustom logos:");
            listAvailableLogos(instance);
            exit(0);
        }
        else if(strcasecmp(subkey, "-logos-autocompletion") == 0)
        {
            ffLogoBuiltinListAutocompletion();
            exit(0);
        }
        else
            goto error;
    }

    ///////////////////
    //General options//
    ///////////////////

    else if(strcasecmp(key, "-r") == 0 || strcasecmp(key, "--recache") == 0)
        instance->config.recache = optionParseBoolean(value);
    else if(strcasecmp(key, "--load-config") == 0)
        optionParseConfigFile(instance, data, key, value);
    else if(strcasecmp(key, "--gen-config") == 0)
        generateConfigFile(instance, false);
    else if(strcasecmp(key, "--gen-config-force") == 0)
        generateConfigFile(instance, true);
    else if(strcasecmp(key, "--thread") == 0 || strcasecmp(key, "--multithreading") == 0)
        instance->config.multithreading = optionParseBoolean(value);
    else if(strcasecmp(key, "--stat") == 0)
    {
        if((instance->config.stat = optionParseBoolean(value)))
            instance->config.showErrors = true;
    }
    else if(strcasecmp(key, "--allow-slow-operations") == 0)
        instance->config.allowSlowOperations = optionParseBoolean(value);
    else if(strcasecmp(key, "--escape-bedrock") == 0)
        instance->config.escapeBedrock = optionParseBoolean(value);
    else if(strcasecmp(key, "--pipe") == 0)
        instance->config.pipe = optionParseBoolean(value);
    else if(strcasecmp(key, "--load-user-config") == 0)
        data->loadUserConfig = optionParseBoolean(value);

    ////////////////
    //Logo options//
    ////////////////

    else if(ffParseLogoCommandOptions(&instance->config.logo, key, value)) {}

    ///////////////////
    //Display options//
    ///////////////////

    else if(strcasecmp(key, "--show-errors") == 0)
        instance->config.showErrors = optionParseBoolean(value);
    else if(strcasecmp(key, "--disable-linewrap") == 0)
        instance->config.disableLinewrap = optionParseBoolean(value);
    else if(strcasecmp(key, "--hide-cursor") == 0)
        instance->config.hideCursor = optionParseBoolean(value);
    else if(strcasecmp(key, "-s") == 0 || strcasecmp(key, "--structure") == 0)
        optionParseString(key, value, &data->structure);
    else if(strcasecmp(key, "--separator") == 0)
        optionParseString(key, value, &instance->config.keyValueSeparator);
    else if(strcasecmp(key, "--color-keys") == 0)
        optionParseColor(key, value, &instance->config.colorKeys);
    else if(strcasecmp(key, "--color-title") == 0)
        optionParseColor(key, value, &instance->config.colorTitle);
    else if(strcasecmp(key, "-c") == 0 || strcasecmp(key, "--color") == 0)
    {
        optionParseColor(key, value, &instance->config.colorKeys);
        ffStrbufSet(&instance->config.colorTitle, &instance->config.colorKeys);
    }
    else if(strcasecmp(key, "--binary-prefix") == 0)
    {
        optionParseEnum(key, value, &instance->config.binaryPrefixType,
            "iec", FF_BINARY_PREFIX_TYPE_IEC,
            "si", FF_BINARY_PREFIX_TYPE_SI,
            "jedec", FF_BINARY_PREFIX_TYPE_JEDEC,
            NULL
        );
    }

    ///////////////////////
    //Module args options//
    ///////////////////////

    else if(ffParseOSCommandOptions(&instance->config.os, key, value)) {}
    else if(ffParseHostCommandOptions(&instance->config.host, key, value)) {}
    else if(ffParseOSCommandOptions(&instance->config.os, key, value)) {}
    else if(ffParseBiosCommandOptions(&instance->config.bios, key, value)) {}
    else if(ffParseBoardCommandOptions(&instance->config.board, key, value)) {}
    else if(optionParseModuleArgs(key, value, "chassis", &instance->config.chassis)) {}
    else if(ffParseCommandCommandOptions(&instance->config.command, key, value)) {}
    else if(ffParseCustomCommandOptions(&instance->config.custom, key, value)) {}
    else if(ffParseKernelCommandOptions(&instance->config.kernel, key, value)) {}
    else if(ffParseUptimeCommandOptions(&instance->config.uptime, key, value)) {}
    else if(optionParseModuleArgs(key, value, "processes", &instance->config.processes)) {}
    else if(ffParsePackagesCommandOptions(&instance->config.packages, key, value)) {}
    else if(ffParseShellCommandOptions(&instance->config.shell, key, value)) {}
    else if(ffParseDisplayCommandOptions(&instance->config.display, key, value)) {}
    else if(ffParseBrightnessCommandOptions(&instance->config.brightness, key, value)) {}
    else if(ffParseDECommandOptions(&instance->config.de, key, value)) {}
    else if(ffParseWifiCommandOptions(&instance->config.wifi, key, value)) {}
    else if(ffParseWMCommandOptions(&instance->config.wm, key, value)) {}
    else if(ffParseWMThemeCommandOptions(&instance->config.wmTheme, key, value)) {}
    else if(ffParseThemeCommandOptions(&instance->config.theme, key, value)) {}
    else if(ffParseIconsCommandOptions(&instance->config.icons, key, value)) {}
    else if(ffParseWallpaperCommandOptions(&instance->config.wallpaper, key, value)) {}
    else if(ffParseFontCommandOptions(&instance->config.font, key, value)) {}
    else if(ffParseCursorCommandOptions(&instance->config.cursor, key, value)) {}
    else if(ffParseTerminalCommandOptions(&instance->config.terminal, key, value)) {}
    else if(ffParseTerminalFontCommandOptions(&instance->config.terminalFont, key, value)) {}
    else if(ffParseCPUCommandOptions(&instance->config.cpu, key, value)) {}
    else if(ffParseCPUUsageCommandOptions(&instance->config.cpuUsage, key, value)) {}
    else if(ffParseGPUCommandOptions(&instance->config.gpu, key, value)) {}
    else if(ffParseMemoryCommandOptions(&instance->config.memory, key, value)) {}
    else if(ffParseSwapCommandOptions(&instance->config.swap, key, value)) {}
    else if(ffParseDiskCommandOptions(&instance->config.disk, key, value)) {}
    else if(ffParseBatteryCommandOptions(&instance->config.battery, key, value)) {}
    else if(ffParsePowerAdapterCommandOptions(&instance->config.powerAdapter, key, value)) {}
    else if(ffParseLocaleCommandOptions(&instance->config.locale, key, value)) {}
    else if(ffParseLocalIpCommandOptions(&instance->config.localIP, key, value)) {}
    else if(ffParsePublicIpCommandOptions(&instance->config.publicIP, key, value)) {}
    else if(ffParseWeatherCommandOptions(&instance->config.weather, key, value)) {}
    else if(ffParsePlayerCommandOptions(&instance->config.player, key, value)) {}
    else if(ffParseMediaCommandOptions(&instance->config.media, key, value)) {}
    else if(ffParseDateTimeCommandOptions(&instance->config.dateTime, key, value)) {}
    else if(ffParseVulkanCommandOptions(&instance->config.vulkan, key, value)) {}
    else if(ffParseOpenGLCommandOptions(&instance->config.openGL, key, value)) {}
    else if(ffParseOpenCLCommandOptions(&instance->config.openCL, key, value)) {}
    else if(ffParseUsersCommandOptions(&instance->config.users, key, value)) {}
    else if(ffParseBluetoothCommandOptions(&instance->config.bluetooth, key, value)) {}
    else if(ffParseSeparatorCommandOptions(&instance->config.separator, key, value)) {}
    else if(ffParseSoundCommandOptions(&instance->config.sound, key, value)) {}
    else if(ffParseGamepadCommandOptions(&instance->config.gamepad, key, value)) {}

    ///////////////////
    //Library options//
    ///////////////////

    else if(startsWith(key, "--lib"))
    {
        const char* subkey = key + strlen("--lib");
        if(strcasecmp(subkey, "-PCI") == 0)
            optionParseString(key, value, &instance->config.libPCI);
        else if(strcasecmp(subkey, "-vulkan") == 0)
            optionParseString(key, value, &instance->config.libVulkan);
        else if(strcasecmp(subkey, "-freetype") == 0)
            optionParseString(key, value, &instance->config.libfreetype);
        else if(strcasecmp(subkey, "-wayland") == 0)
            optionParseString(key, value, &instance->config.libWayland);
        else if(strcasecmp(subkey, "-xcb-randr") == 0)
            optionParseString(key, value, &instance->config.libXcbRandr);
        else if(strcasecmp(subkey, "-xcb") == 0)
            optionParseString(key, value, &instance->config.libXcb);
        else if(strcasecmp(subkey, "-Xrandr") == 0)
            optionParseString(key, value, &instance->config.libXrandr);
        else if(strcasecmp(subkey, "-X11") == 0)
            optionParseString(key, value, &instance->config.libX11);
        else if(strcasecmp(subkey, "-gio") == 0)
            optionParseString(key, value, &instance->config.libGIO);
        else if(strcasecmp(subkey, "-DConf") == 0)
            optionParseString(key, value, &instance->config.libDConf);
        else if(strcasecmp(subkey, "-dbus") == 0)
            optionParseString(key, value, &instance->config.libDBus);
        else if(strcasecmp(subkey, "-XFConf") == 0)
            optionParseString(key, value, &instance->config.libXFConf);
        else if(strcasecmp(subkey, "-sqlite") == 0 || strcasecmp(subkey, "-sqlite3") == 0)
            optionParseString(key, value, &instance->config.libSQLite3);
        else if(strcasecmp(subkey, "-rpm") == 0)
            optionParseString(key, value, &instance->config.librpm);
        else if(strcasecmp(subkey, "-imagemagick") == 0)
            optionParseString(key, value, &instance->config.libImageMagick);
        else if(strcasecmp(subkey, "-z") == 0)
            optionParseString(key, value, &instance->config.libZ);
        else if(strcasecmp(subkey, "-chafa") == 0)
            optionParseString(key, value, &instance->config.libChafa);
        else if(strcasecmp(subkey, "-egl") == 0)
            optionParseString(key, value, &instance->config.libEGL);
        else if(strcasecmp(subkey, "-glx") == 0)
            optionParseString(key, value, &instance->config.libGLX);
        else if(strcasecmp(subkey, "-osmesa") == 0)
            optionParseString(key, value, &instance->config.libOSMesa);
        else if(strcasecmp(subkey, "-opencl") == 0)
            optionParseString(key, value, &instance->config.libOpenCL);
        else if(strcasecmp(subkey, "-jsonc") == 0)
            optionParseString(key, value, &instance->config.libJSONC);
        else if(strcasecmp(subkey, "-wlanapi") == 0)
            optionParseString(key, value, &instance->config.libwlanapi);
        else if(strcasecmp(key, "-pulse") == 0)
            optionParseString(key, value, &instance->config.libPulse);
        else if(strcasecmp(subkey, "-nm") == 0)
            optionParseString(key, value, &instance->config.libnm);
        else
            goto error;
    }

    //////////////////
    //Module options//
    //////////////////

    else if(strcasecmp(key, "--percent-type") == 0)
        instance->config.percentType = optionParseUInt32(key, value);

    //////////////////
    //Unknown option//
    //////////////////

    else
    {
error:
        fprintf(stderr, "Error: unknown option: %s\n", key);
        exit(400);
    }
}

static void parseConfigFiles(FFinstance* instance, FFdata* data)
{
    for(uint32_t i = instance->state.platform.configDirs.length; i > 0; --i)
    {
        if(!data->loadUserConfig)
            return;

        FFstrbuf* dir = ffListGet(&instance->state.platform.configDirs, i - 1);
        uint32_t dirLength = dir->length;

        ffStrbufAppendS(dir, "fastfetch/config.conf");
        parseConfigFile(instance, data, dir->chars);
        ffStrbufSubstrBefore(dir, dirLength);
    }
}

static void parseArguments(FFinstance* instance, FFdata* data, int argc, const char** argv)
{
    if(!data->loadUserConfig)
        return;

    for(int i = 1; i < argc; i++)
    {
        if(i == argc - 1 || (
            *argv[i + 1] == '-' &&
            strcasecmp(argv[i], "--separator-string") != 0 // Separator string can start with a -
        )) {
            parseOption(instance, data, argv[i], NULL);
        }
        else
        {
            parseOption(instance, data, argv[i], argv[i + 1]);
            ++i;
        }
    }
}

static void parseStructureCommand(FFinstance* instance, const char* line)
{
    if(strcasecmp(line, FF_BREAK_MODULE_NAME) == 0)
        ffPrintBreak(instance);
    else if(strcasecmp(line, FF_TITLE_MODULE_NAME) == 0)
        ffPrintTitle(instance, &instance->config.title);
    else if(strcasecmp(line, FF_SEPARATOR_MODULE_NAME) == 0)
        ffPrintSeparator(instance, &instance->config.separator);
    else if(strcasecmp(line, FF_OS_MODULE_NAME) == 0)
        ffPrintOS(instance, &instance->config.os);
    else if(strcasecmp(line, FF_HOST_MODULE_NAME) == 0)
        ffPrintHost(instance, &instance->config.host);
    else if(strcasecmp(line, FF_BIOS_MODULE_NAME) == 0)
        ffPrintBios(instance, &instance->config.bios);
    else if(strcasecmp(line, FF_BOARD_MODULE_NAME) == 0)
        ffPrintBoard(instance, &instance->config.board);
    else if(strcasecmp(line, FF_BRIGHTNESS_MODULE_NAME) == 0)
        ffPrintBrightness(instance, &instance->config.brightness);
    else if(strcasecmp(line, "chassis") == 0)
        ffPrintChassis(instance);
    else if(strcasecmp(line, FF_KERNEL_MODULE_NAME) == 0)
        ffPrintKernel(instance, &instance->config.kernel);
    else if(strcasecmp(line, "processes") == 0)
        ffPrintProcesses(instance);
    else if(strcasecmp(line, FF_UPTIME_MODULE_NAME) == 0)
        ffPrintUptime(instance, &instance->config.uptime);
    else if(strcasecmp(line, FF_PACKAGES_MODULE_NAME) == 0)
        ffPrintPackages(instance, &instance->config.packages);
    else if(strcasecmp(line, FF_SHELL_MODULE_NAME) == 0)
        ffPrintShell(instance, &instance->config.shell);
    else if(strcasecmp(line, FF_DISPLAY_MODULE_NAME) == 0)
        ffPrintDisplay(instance, &instance->config.display);
    else if(strcasecmp(line, FF_DE_MODULE_NAME) == 0)
        ffPrintDE(instance, &instance->config.de);
    else if(strcasecmp(line, FF_WM_MODULE_NAME) == 0)
        ffPrintWM(instance, &instance->config.wm);
    else if(strcasecmp(line, FF_THEME_MODULE_NAME) == 0)
        ffPrintTheme(instance, &instance->config.theme);
    else if(strcasecmp(line, FF_WMTHEME_MODULE_NAME) == 0)
        ffPrintWMTheme(instance, &instance->config.wmTheme);
    else if(strcasecmp(line, FF_ICONS_MODULE_NAME) == 0)
        ffPrintIcons(instance, &instance->config.icons);
    else if(strcasecmp(line, FF_WALLPAPER_MODULE_NAME) == 0)
        ffPrintWallpaper(instance, &instance->config.wallpaper);
    else if(strcasecmp(line, FF_FONT_MODULE_NAME) == 0)
        ffPrintFont(instance, &instance->config.font);
    else if(strcasecmp(line, FF_CURSOR_MODULE_NAME) == 0)
        ffPrintCursor(instance, &instance->config.cursor);
    else if(strcasecmp(line, FF_TERMINAL_MODULE_NAME) == 0)
        ffPrintTerminal(instance, &instance->config.terminal);
    else if(strcasecmp(line, FF_TERMINALFONT_MODULE_NAME) == 0)
        ffPrintTerminalFont(instance, &instance->config.terminalFont);
    else if(strcasecmp(line, FF_CPU_MODULE_NAME) == 0)
        ffPrintCPU(instance, &instance->config.cpu);
    else if(strcasecmp(line, FF_CPUUSAGE_MODULE_NAME) == 0)
        ffPrintCPUUsage(instance, &instance->config.cpuUsage);
    else if(strcasecmp(line, FF_CUSTOM_MODULE_NAME) == 0)
        ffPrintCustom(instance, &instance->config.custom);
    else if(strcasecmp(line, FF_GPU_MODULE_NAME) == 0)
        ffPrintGPU(instance, &instance->config.gpu);
    else if(strcasecmp(line, FF_MEMORY_MODULE_NAME) == 0)
        ffPrintMemory(instance, &instance->config.memory);
    else if(strcasecmp(line, FF_SWAP_MODULE_NAME) == 0)
        ffPrintSwap(instance, &instance->config.swap);
    else if(strcasecmp(line, FF_DISK_MODULE_NAME) == 0)
        ffPrintDisk(instance, &instance->config.disk);
    else if(strcasecmp(line, FF_BATTERY_MODULE_NAME) == 0)
        ffPrintBattery(instance, &instance->config.battery);
    else if(strcasecmp(line, FF_POWERADAPTER_MODULE_NAME) == 0)
        ffPrintPowerAdapter(instance, &instance->config.powerAdapter);
    else if(strcasecmp(line, FF_LOCALE_MODULE_NAME) == 0)
        ffPrintLocale(instance, &instance->config.locale);
    else if(strcasecmp(line, "localip") == 0)
        ffPrintLocalIp(instance, &instance->config.localIP);
    else if(strcasecmp(line, FF_PUBLICIP_MODULE_NAME) == 0)
        ffPrintPublicIp(instance, &instance->config.publicIP);
    else if(strcasecmp(line, FF_WIFI_MODULE_NAME) == 0)
        ffPrintWifi(instance, &instance->config.wifi);
    else if(strcasecmp(line, FF_WEATHER_MODULE_NAME) == 0)
        ffPrintWeather(instance, &instance->config.weather);
    else if(strcasecmp(line, FF_PLAYER_MODULE_NAME) == 0)
        ffPrintPlayer(instance, &instance->config.player);
    else if(strcasecmp(line, FF_MEDIA_MODULE_NAME) == 0)
        ffPrintMedia(instance, &instance->config.media);
    else if(strcasecmp(line, FF_DATETIME_MODULE_NAME) == 0)
        ffPrintDateTime(instance, &instance->config.dateTime);
    else if(strcasecmp(line, FF_COLORS_MODULE_NAME) == 0)
        ffPrintColors(instance);
    else if(strcasecmp(line, FF_VULKAN_MODULE_NAME) == 0)
        ffPrintVulkan(instance, &instance->config.vulkan);
    else if(strcasecmp(line, FF_OPENGL_MODULE_NAME) == 0)
        ffPrintOpenGL(instance, &instance->config.openGL);
    else if(strcasecmp(line, FF_OPENCL_MODULE_NAME) == 0)
        ffPrintOpenCL(instance, &instance->config.openCL);
    else if(strcasecmp(line, FF_USERS_MODULE_NAME) == 0)
        ffPrintUsers(instance, &instance->config.users);
    else if(strcasecmp(line, FF_COMMAND_MODULE_NAME) == 0)
        ffPrintCommand(instance, &instance->config.command);
    else if(strcasecmp(line, FF_BLUETOOTH_MODULE_NAME) == 0)
        ffPrintBluetooth(instance, &instance->config.bluetooth);
    else if(strcasecmp(line, FF_SOUND_MODULE_NAME) == 0)
        ffPrintSound(instance, &instance->config.sound);
    else if(strcasecmp(line, FF_GAMEPAD_MODULE_NAME) == 0)
        ffPrintGamepad(instance, &instance->config.gamepad);
    else if(strcasecmp(line, FF_JSONCONFIG_MODULE_NAME) == 0)
        ffPrintJsonConfig(instance);
    else
        ffPrintErrorString(instance, line, 0, NULL, NULL, "<no implementation provided>");
}

int main(int argc, const char** argv)
{
    FFinstance instance;
    ffInitInstance(&instance);

    //Data stores things only needed for the configuration of fastfetch
    FFdata data;
    ffStrbufInitA(&data.structure, 256);
    data.loadUserConfig = true;

    if(!getenv("NO_CONFIG"))
        parseConfigFiles(&instance, &data);
    parseArguments(&instance, &data, argc, argv);

    //If we don't have a custom structure, use the default one
    if(data.structure.length == 0)
        ffStrbufAppendS(&data.structure, FASTFETCH_DATATEXT_STRUCTURE);

    if(ffStrbufContainIgnCaseS(&data.structure, FF_CPUUSAGE_MODULE_NAME))
        ffPrepareCPUUsage();

    if(instance.config.multithreading)
    {
        if(ffStrbufContainIgnCaseS(&data.structure, FF_PUBLICIP_MODULE_NAME))
            ffPreparePublicIp(&instance.config.publicIP);

        if(ffStrbufContainIgnCaseS(&data.structure, FF_WEATHER_MODULE_NAME))
            ffPrepareWeather(&instance.config.weather);
    }

    ffStart(&instance);

    #if defined(_WIN32) && defined(FF_ENABLE_BUFFER)
        fflush(stdout);
    #endif

    //Parse the structure and call the modules
    uint32_t startIndex = 0;
    while (startIndex < data.structure.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&data.structure, startIndex, ':');
        data.structure.chars[colonIndex] = '\0';

        uint64_t ms = 0;
        if(__builtin_expect(instance.config.stat, false))
            ms = ffTimeGetTick();

        parseStructureCommand(&instance, data.structure.chars + startIndex);

        if(__builtin_expect(instance.config.stat, false))
        {
            char str[32];
            int len = snprintf(str, sizeof str, "%" PRIu64 "ms", ffTimeGetTick() - ms);
            if(instance.config.pipe)
                puts(str);
            else
                printf("\033[s\033[1A\033[9999999C\033[%dD%s\033[u", len, str); // Save; Up 1; Right 9999999; Left <len>; Print <str>; Load
        }

        #if defined(_WIN32) && defined(FF_ENABLE_BUFFER)
            fflush(stdout);
        #endif

        startIndex = colonIndex + 1;
    }

    ffFinish(&instance);

    ffStrbufDestroy(&data.structure);

    ffDestroyInstance(&instance);
}
