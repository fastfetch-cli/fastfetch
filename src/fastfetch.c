#include "fastfetch.h"
#include "util/FFvaluestore.h"
#include "common/printing.h"
#include "common/parsing.h"
#include "common/io.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef WIN32
    #include "util/windows/getline.h"
    #include <fileapi.h>
#else
    #include <dirent.h>
#endif

typedef struct CustomValue
{
    bool printKey;
    FFstrbuf value;
} CustomValue;

// Things only needed by fastfetch
typedef struct FFdata
{
    FFvaluestore customValues;
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
        fputs(FASTFETCH_DATATEXT_HELP, stdout);
    else if(strcasecmp(command, "c") == 0 || strcasecmp(command, "color") == 0)
        fputs(FASTFETCH_DATATEXT_HELP_COLOR, stdout);
    else if(strcasecmp(command, "format") == 0)
        fputs(FASTFETCH_DATATEXT_HELP_FORMAT, stdout);
    else if(strcasecmp(command, "load-config") == 0 || strcasecmp(command, "loadconfig") == 0 || strcasecmp(command, "config") == 0)
        fputs(FASTFETCH_DATATEXT_HELP_CONFIG, stdout);
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
        constructAndPrintCommandHelpFormat("packages", "{2} (pacman){?3}[{3}]{?}, {4} (dpkg), {5} (rpm), {6} (emerge), {7} (xbps), {8} (nix-system), {9} (nix-user), {10} (nix-default), {11} (apk), {12} (pkg), {13} (flatpak), {14} (snap), {15} (brew), {16} (port), {17} (scoop)", 17,
            "Number of all packages",
            "Number of pacman packages",
            "Pacman branch on manjaro",
            "Number of dpkg packages",
            "Number of rpm packages",
            "Number of emerge packages",
            "Number of xbps packages",
            "Number of nix-system packages",
            "Number of nix-user packages",
            "Number of nix-default packages",
            "Number of apk packages",
            "Number of pkg packages",
            "Number of flatpak packages",
            "Number of snap packages",
            "Number of brew packages",
            "Number of macports packages"
            "Number of scoop packages"
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
        constructAndPrintCommandHelpFormat("gpu", "{} {}", 5,
            "GPU vendor",
            "GPU name",
            "GPU driver",
            "GPU temperature",
            "GPU core count"
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
            "Connection PHY type",
            "Connection signal quality (percentage)",
            "Connection RX rate",
            "Connection TX rate",
            "Security enabled",
            "Security 802.1X enabled",
            "Security algorithm"
        );
    }
    else if(strcasecmp(command, "player-format") == 0)
    {
        constructAndPrintCommandHelpFormat("player", "{}", 4,
            "Pretty player name",
            "Player name",
            "DBus bus name",
            "URL name"
        );
    }
    else if(strcasecmp(command, "song-format") == 0 || strcasecmp(command, "media-format") == 0)
    {
        constructAndPrintCommandHelpFormat("song", "{3} - {1}", 4,
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
    else
        fprintf(stderr, "No specific help for command %s provided\n", command);
}

static inline void listAvailablePresetsFromFolder(FFstrbuf* folder, uint8_t indentation, const char* folderName)
{
    #ifndef _WIN32
    DIR* dir = opendir(folder->chars);
    if(dir == NULL)
        return;

    uint32_t folderLength = folder->length;

    if(folderName != NULL)
        printf("%s/\n", folderName);

    struct dirent* entry;

    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_type == DT_DIR)
        {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            ffStrbufAppendS(folder, entry->d_name);
            ffStrbufAppendC(folder, '/');
            listAvailablePresetsFromFolder(folder, (uint8_t) (indentation + 1), entry->d_name);
            ffStrbufSubstrBefore(folder, folderLength);
            continue;
        }

        for(uint8_t i = 0; i < indentation; i++)
            fputs("  | ", stdout);

        puts(entry->d_name);
    }

    closedir(dir);
    #else
    uint32_t folderLength = folder->length;

    if(folderName != NULL)
        printf("%s/\n", folderName);

    ffStrbufAppendC(folder, '*');
    WIN32_FIND_DATAA entry;
    HANDLE hFind = FindFirstFileA(folder->chars, &entry);
    if(hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if(strcmp(entry.cFileName, ".") == 0 || strcmp(entry.cFileName, "..") == 0)
                continue;

            ffStrbufSubstrBefore(folder, folderLength);
            ffStrbufAppendS(folder, entry.cFileName);
            ffStrbufAppendC(folder, '/');
            listAvailablePresetsFromFolder(folder, (uint8_t) (indentation + 1), entry.cFileName);
            ffStrbufSubstrBefore(folder, folderLength);
            continue;
        }

        for(uint8_t i = 0; i < indentation; i++)
            fputs("  | ", stdout);

        puts(entry.cFileName);
    } while (FindNextFileA(hFind, &entry));
    FindClose(hFind);
    #endif
}

static inline void listAvailablePresets(FFinstance* instance)
{
    FFstrbuf folder;
    ffStrbufInitA(&folder, 64);

    ffStrbufSetS(&folder, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&folder, "/.local/share/fastfetch/presets/");
    listAvailablePresetsFromFolder(&folder, 0, NULL);

    ffStrbufSetS(&folder, FASTFETCH_TARGET_DIR_USR"/share/fastfetch/presets/");
    listAvailablePresetsFromFolder(&folder, 0, NULL);

    ffStrbufDestroy(&folder);
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

    //Try to load as an user preset

    FFstrbuf filename;
    ffStrbufInitA(&filename, 64);

    ffStrbufAppendS(&filename, instance->state.passwd->pw_dir);
    ffStrbufAppendS(&filename, "/.local/share/fastfetch/presets/");
    ffStrbufAppendS(&filename, value);

    if(parseConfigFile(instance, data, filename.chars))
    {
        ffStrbufDestroy(&filename);
        return;
    }


    //Try to load as a system preset

    ffStrbufSetS(&filename, FASTFETCH_TARGET_DIR_USR"/share/fastfetch/presets/");
    ffStrbufAppendS(&filename, value);

    if(parseConfigFile(instance, data, filename.chars))
    {
        ffStrbufDestroy(&filename);
        return;
    }

    //File not found

    fprintf(stderr, "Error: couldn't find config: %s\n", value);
    ffStrbufDestroy(&filename);
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

static void optionParseCustomValue(FFdata* data, const char* key, const char* value, bool printKey)
{
    if(value == NULL)
    {
        fprintf(stderr, "Error: usage: %s <key=value>\n", key);
        exit(411);
    }

    char* separator = strchr(value, '=');

    if(separator == NULL)
    {
        fprintf(stderr, "Error: usage: %s <key=value>, '=' missing\n", key);
        exit(412);
    }

    *separator = '\0';

    bool created;
    CustomValue* customValue = ffValuestoreSet(&data->customValues, value, &created);
    if(created)
        ffStrbufInit(&customValue->value);
    ffStrbufSetS(&customValue->value, separator + 1);
    customValue->printKey = printKey;
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
        puts("fastfetch "FASTFETCH_PROJECT_VERSION""FASTFETCH_PROJECT_VERSION_TWEAK);
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
        else if(strcasecmp(subkey, "-features") == 0)
        {
            ffListFeatures();
            exit(0);
        }
        else if(strcasecmp(subkey, "-logos") == 0)
        {
            ffLogoBuiltinList();
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
    {
        //Set cacheSave as well, because the user expects the values to be cached when expliciting using --recache
        instance->config.recache = optionParseBoolean(value);
        instance->config.cacheSave = instance->config.recache;
    }
    else if(strcasecmp(key, "--nocache") == 0)
    {
        instance->config.recache = optionParseBoolean(value);
        instance->config.cacheSave = false;
    }
    else if(strcasecmp(key, "--load-config") == 0)
        optionParseConfigFile(instance, data, key, value);
    else if(strcasecmp(key, "--thread") == 0 || strcasecmp(key, "--multithreading") == 0)
        instance->config.multithreading = optionParseBoolean(value);
    else if(strcasecmp(key, "--allow-slow-operations") == 0)
        instance->config.allowSlowOperations = optionParseBoolean(value);
    else if(strcasecmp(key, "--disable-stdout-buffer") == 0)
    {
        if(optionParseBoolean(value))
            setvbuf(stdout, NULL, _IONBF, 0);
    }
    else if(strcasecmp(key, "--escape-bedrock") == 0)
        instance->config.escapeBedrock = optionParseBoolean(value);
    else if(strcasecmp(key, "--pipe") == 0)
        instance->config.pipe = optionParseBoolean(value);
    else if(strcasecmp(key, "--load-user-config") == 0)
        data->loadUserConfig = optionParseBoolean(value);

    ////////////////
    //Logo options//
    ////////////////

    else if(strcasecmp(key, "-l") == 0 || strcasecmp(key, "--logo") == 0)
    {
        optionParseString(key, value, &instance->config.logo.source);

        //this is usally wanted when using the none logo
        if(strcasecmp(value, "none") == 0)
        {
            instance->config.logo.paddingRight = 0;
            instance->config.logo.paddingLeft = 0;
        }
    }
    else if(startsWith(key, "--logo"))
    {
        const char* subkey = key + strlen("--logo");
        if(strcasecmp(subkey, "-type") == 0)
        {
            optionParseEnum(key, value, &instance->config.logo.type,
                "auto", FF_LOGO_TYPE_AUTO,
                "builtin", FF_LOGO_TYPE_BUILTIN,
                "file", FF_LOGO_TYPE_FILE,
                "raw", FF_LOGO_TYPE_RAW,
                "sixel", FF_LOGO_TYPE_SIXEL,
                "kitty", FF_LOGO_TYPE_KITTY,
                "chafa", FF_LOGO_TYPE_CHAFA,
                NULL
            );
        }
        else if(startsWith(subkey, "-color-") && key[13] != '\0' && key[14] == '\0') // matches "--logo-color-*"
        {
            //Map the number to an array index, so that '1' -> 0, '2' -> 1, etc.
            int index = (int)key[13] - 49;

            //Match only --logo-color-[1-9]
            if(index < 0 || index >= FASTFETCH_LOGO_MAX_COLORS)
            {
                fprintf(stderr, "Error: invalid --color-[1-9] index: %c\n", key[13]);
                exit(472);
            }

            optionParseColor(key, value, &instance->config.logo.colors[index]);
        }
        else if(strcasecmp(subkey, "-width") == 0)
            instance->config.logo.width = optionParseUInt32(key, value);
        else if(strcasecmp(subkey, "-height") == 0)
            instance->config.logo.height = optionParseUInt32(key, value);
        else if(strcasecmp(subkey, "-padding") == 0)
        {
            uint32_t padding = optionParseUInt32(key, value);
            instance->config.logo.paddingLeft = padding;
            instance->config.logo.paddingRight = padding;
        }
        else if(strcasecmp(subkey, "-padding-left") == 0)
            instance->config.logo.paddingLeft = optionParseUInt32(key, value);
        else if(strcasecmp(subkey, "-padding-right") == 0)
            instance->config.logo.paddingRight = optionParseUInt32(key, value);
        else if(strcasecmp(subkey, "-print-remaining") == 0)
            instance->config.logo.printRemaining = optionParseBoolean(value);
        else
            goto error;
    }
    else if(strcasecmp(key, "--sixel") == 0)
    {
        optionParseString(key, value, &instance->config.logo.source);
        instance->config.logo.type = FF_LOGO_TYPE_SIXEL;
    }
    else if(strcasecmp(key, "--kitty") == 0)
    {
        optionParseString(key, value, &instance->config.logo.source);
        instance->config.logo.type = FF_LOGO_TYPE_KITTY;
    }
    else if(strcasecmp(key, "--file") == 0)
    {
        optionParseString(key, value, &instance->config.logo.source);
        instance->config.logo.type = FF_LOGO_TYPE_FILE;
    }
    else if(strcasecmp(key, "--raw") == 0)
    {
        optionParseString(key, value, &instance->config.logo.source);
        instance->config.logo.type = FF_LOGO_TYPE_RAW;
    }
    else if(strcasecmp(key, "--chafa") == 0)
    {
        optionParseString(key, value, &instance->config.logo.source);
        instance->config.logo.type = FF_LOGO_TYPE_CHAFA;
    }

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
        optionParseString(key, value, &instance->config.separator);
    else if(strcasecmp(key, "--color-keys") == 0)
        optionParseColor(key, value, &instance->config.colorKeys);
    else if(strcasecmp(key, "--color-title") == 0)
        optionParseColor(key, value, &instance->config.colorTitle);
    else if(strcasecmp(key, "-c") == 0 || strcasecmp(key, "--color") == 0)
    {
        optionParseColor(key, value, &instance->config.colorKeys);
        ffStrbufSet(&instance->config.colorTitle, &instance->config.colorKeys);
    }
    else if(strcasecmp(key, "--set") == 0)
        optionParseCustomValue(data, key, value, true);
    else if(strcasecmp(key, "--set-keyless") == 0)
        optionParseCustomValue(data, key, value, false);
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

    else if(optionParseModuleArgs(key, value, "os", &instance->config.os)) {}
    else if(optionParseModuleArgs(key, value, "host", &instance->config.host)) {}
    else if(optionParseModuleArgs(key, value, "bios", &instance->config.bios)) {}
    else if(optionParseModuleArgs(key, value, "board", &instance->config.board)) {}
    else if(optionParseModuleArgs(key, value, "kernel", &instance->config.kernel)) {}
    else if(optionParseModuleArgs(key, value, "uptime", &instance->config.uptime)) {}
    else if(optionParseModuleArgs(key, value, "processes", &instance->config.processes)) {}
    else if(optionParseModuleArgs(key, value, "packages", &instance->config.packages)) {}
    else if(optionParseModuleArgs(key, value, "shell", &instance->config.shell)) {}
    else if(optionParseModuleArgs(key, value, "resolution", &instance->config.resolution)) {}
    else if(optionParseModuleArgs(key, value, "de", &instance->config.de)) {}
    else if(optionParseModuleArgs(key, value, "wifi", &instance->config.wifi)) {}
    else if(optionParseModuleArgs(key, value, "wm", &instance->config.wm)) {}
    else if(optionParseModuleArgs(key, value, "wm-theme", &instance->config.wmTheme)) {}
    else if(optionParseModuleArgs(key, value, "theme", &instance->config.theme)) {}
    else if(optionParseModuleArgs(key, value, "icons", &instance->config.icons)) {}
    else if(optionParseModuleArgs(key, value, "font", &instance->config.font)) {}
    else if(optionParseModuleArgs(key, value, "cursor", &instance->config.cursor)) {}
    else if(optionParseModuleArgs(key, value, "terminal", &instance->config.terminal)) {}
    else if(optionParseModuleArgs(key, value, "terminal-font", &instance->config.terminalFont)) {}
    else if(optionParseModuleArgs(key, value, "cpu", &instance->config.cpu)) {}
    else if(optionParseModuleArgs(key, value, "cpu-usage", &instance->config.cpuUsage)) {}
    else if(optionParseModuleArgs(key, value, "gpu", &instance->config.gpu)) {}
    else if(optionParseModuleArgs(key, value, "memory", &instance->config.memory)) {}
    else if(optionParseModuleArgs(key, value, "swap", &instance->config.swap)) {}
    else if(optionParseModuleArgs(key, value, "disk", &instance->config.disk)) {}
    else if(optionParseModuleArgs(key, value, "battery", &instance->config.battery)) {}
    else if(optionParseModuleArgs(key, value, "poweradapter", &instance->config.powerAdapter)) {}
    else if(optionParseModuleArgs(key, value, "locale", &instance->config.locale)) {}
    else if(optionParseModuleArgs(key, value, "local-ip", &instance->config.localIP)) {}
    else if(optionParseModuleArgs(key, value, "public-ip", &instance->config.publicIP)) {}
    else if(optionParseModuleArgs(key, value, "weather", &instance->config.weather)) {}
    else if(optionParseModuleArgs(key, value, "player", &instance->config.player)) {}
    else if(optionParseModuleArgs(key, value, "song", &instance->config.song)) {}
    else if(optionParseModuleArgs(key, value, "datetime", &instance->config.dateTime)) {}
    else if(optionParseModuleArgs(key, value, "date", &instance->config.date)) {}
    else if(optionParseModuleArgs(key, value, "time", &instance->config.time)) {}
    else if(optionParseModuleArgs(key, value, "vulkan", &instance->config.vulkan)) {}
    else if(optionParseModuleArgs(key, value, "opengl", &instance->config.openGL)) {}
    else if(optionParseModuleArgs(key, value, "opencl", &instance->config.openCL)) {}
    else if(optionParseModuleArgs(key, value, "users", &instance->config.users)) {}

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
        else if(strcasecmp(subkey, "-cjson") == 0)
            optionParseString(key, value, &instance->config.libcJSON);
        else if(strcasecmp(subkey, "-wlanapi") == 0)
            optionParseString(key, value, &instance->config.libwlanapi);
        else
            goto error;
    }

    //////////////////
    //Module options//
    //////////////////

    else if(strcasecmp(key, "--cpu-temp") == 0)
        instance->config.cpuTemp = optionParseBoolean(value);
    else if(strcasecmp(key, "--gpu-temp") == 0)
        instance->config.gpuTemp = optionParseBoolean(value);
    else if(strcasecmp(key, "--battery-temp") == 0)
        instance->config.batteryTemp = optionParseBoolean(value);
    else if(strcasecmp(key, "--title-fqdn") == 0)
        instance->config.titleFQDN = optionParseBoolean(value);
    else if(strcasecmp(key, "--disk-folders") == 0)
        optionParseString(key, value, &instance->config.diskFolders);
    else if(strcasecmp(key, "--disk-show-removable") == 0)
        instance->config.diskShowRemovable = optionParseBoolean(value);
    else if(strcasecmp(key, "--disk-show-hidden") == 0)
        instance->config.diskShowHidden = optionParseBoolean(value);
    else if(strcasecmp(key, "--battery-dir") == 0)
        optionParseString(key, value, &instance->config.batteryDir);
    else if(strcasecmp(key, "--separator-string") == 0)
        optionParseString(key, value, &instance->config.separatorString);
    else if(strcasecmp(key, "--localip-show-ipv4") == 0)
        instance->config.localIpShowIpV4 = optionParseBoolean(value);
    else if(strcasecmp(key, "--localip-show-ipv6") == 0)
        instance->config.localIpShowIpV6 = optionParseBoolean(value);
    else if(strcasecmp(key, "--localip-show-loop") == 0)
        instance->config.localIpShowLoop = optionParseBoolean(value);
    else if(strcasecmp(key, "--localip-name-prefix") == 0)
        optionParseString(key, value, &instance->config.localIpNamePrefix);
    else if(strcasecmp(key, "--os-file") == 0)
        optionParseString(key, value, &instance->config.osFile);
    else if(strcasecmp(key, "--player-name") == 0)
        optionParseString(key, value, &instance->config.playerName);
    else if(strcasecmp(key, "--public-ip-url") == 0)
        optionParseString(key, value, &instance->config.publicIpUrl);
    else if(strcasecmp(key, "--public-ip-timeout") == 0)
        instance->config.publicIpTimeout = optionParseUInt32(key, value);
    else if(strcasecmp(key, "--weather-output-format") == 0)
        optionParseString(key, value, &instance->config.weatherOutputFormat);
    else if(strcasecmp(key, "--weather-timeout") == 0)
        instance->config.weatherTimeout = optionParseUInt32(key, value);
    else if(strcasecmp(key, "--gl") == 0)
    {
        optionParseEnum(key, value, &instance->config.glType,
            "auto", FF_GL_TYPE_AUTO,
            "egl", FF_GL_TYPE_EGL,
            "glx", FF_GL_TYPE_GLX,
            "osmesa", FF_GL_TYPE_OSMESA,
            NULL
        );
    }
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

static void parseConfigFileSystem(FFinstance* instance, FFdata* data)
{
    parseConfigFile(instance, data, FASTFETCH_TARGET_DIR_INSTALL_SYSCONF"/fastfetch/config.conf");
}

static void parseConfigFileUser(FFinstance* instance, FFdata* data)
{
    if(!data->loadUserConfig)
        return;

    FFstrbuf* filename = ffListGet(&instance->state.configDirs, 0);
    uint32_t filenameLength = filename->length;

    ffStrbufAppendS(filename, "/fastfetch/config.conf");

    if(!parseConfigFile(instance, data, filename->chars))
        ffWriteFileData(filename->chars, sizeof(FASTFETCH_DATATEXT_CONFIG_USER), FASTFETCH_DATATEXT_CONFIG_USER);

    ffStrbufSubstrBefore(filename, filenameLength);
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

static void parseStructureCommand(FFinstance* instance, FFdata* data, const char* line)
{
    CustomValue* customValue = ffValuestoreGet(&data->customValues, line);
    if(customValue != NULL)
    {
        ffPrintCustom(instance, customValue->printKey ? line : NULL, customValue->value.chars);
        return;
    }

    if(strcasecmp(line, "break") == 0)
        ffPrintBreak(instance);
    else if(strcasecmp(line, "title") == 0)
        ffPrintTitle(instance);
    else if(strcasecmp(line, "separator") == 0)
        ffPrintSeparator(instance);
    else if(strcasecmp(line, "os") == 0)
        ffPrintOS(instance);
    else if(strcasecmp(line, "host") == 0)
        ffPrintHost(instance);
    else if(strcasecmp(line, "bios") == 0)
        ffPrintBios(instance);
    else if(strcasecmp(line, "board") == 0)
        ffPrintBoard(instance);
    else if(strcasecmp(line, "kernel") == 0)
        ffPrintKernel(instance);
    else if(strcasecmp(line, "uptime") == 0)
        ffPrintUptime(instance);
    else if(strcasecmp(line, "processes") == 0)
        ffPrintProcesses(instance);
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
    else if(strcasecmp(line, "cursor") == 0)
        ffPrintCursor(instance);
    else if(strcasecmp(line, "terminal") == 0)
        ffPrintTerminal(instance);
    else if(strcasecmp(line, "terminalfont") == 0)
        ffPrintTerminalFont(instance);
    else if(strcasecmp(line, "cpu") == 0)
        ffPrintCPU(instance);
    else if(strcasecmp(line, "cpuusage") == 0)
        ffPrintCPUUsage(instance);
    else if(strcasecmp(line, "gpu") == 0)
        ffPrintGPU(instance);
    else if(strcasecmp(line, "memory") == 0)
        ffPrintMemory(instance);
    else if(strcasecmp(line, "swap") == 0)
        ffPrintSwap(instance);
    else if(strcasecmp(line, "disk") == 0)
        ffPrintDisk(instance);
    else if(strcasecmp(line, "battery") == 0)
        ffPrintBattery(instance);
    else if(strcasecmp(line, "poweradapter") == 0)
        ffPrintPowerAdapter(instance);
    else if(strcasecmp(line, "locale") == 0)
        ffPrintLocale(instance);
    else if(strcasecmp(line, "localip") == 0)
        ffPrintLocalIp(instance);
    else if(strcasecmp(line, "publicip") == 0)
        ffPrintPublicIp(instance);
    else if(strcasecmp(line, "wifi") == 0)
        ffPrintWifi(instance);
    else if(strcasecmp(line, "weather") == 0)
        ffPrintWeather(instance);
    else if(strcasecmp(line, "player") == 0)
        ffPrintPlayer(instance);
    else if(strcasecmp(line, "media") == 0 || strcasecmp(line, "song") == 0)
        ffPrintSong(instance);
    else if(strcasecmp(line, "datetime") == 0)
        ffPrintDateTime(instance);
    else if(strcasecmp(line, "date") == 0)
        ffPrintDate(instance);
    else if(strcasecmp(line, "time") == 0)
        ffPrintTime(instance);
    else if(strcasecmp(line, "colors") == 0)
        ffPrintColors(instance);
    else if(strcasecmp(line, "vulkan") == 0)
        ffPrintVulkan(instance);
    else if(strcasecmp(line, "opengl") == 0)
        ffPrintOpenGL(instance);
    else if(strcasecmp(line, "opencl") == 0)
        ffPrintOpenCL(instance);
    else if(strcasecmp(line, "users") == 0)
        ffPrintUsers(instance);
    else
        ffPrintErrorString(instance, line, 0, NULL, NULL, "<no implementation provided>");
}

int main(int argc, const char** argv)
{
    FFinstance instance;
    ffInitInstance(&instance);

    //Data stores things only needed for the configuration of fastfetch
    FFdata data;
    ffValuestoreInit(&data.customValues, sizeof(CustomValue));
    ffStrbufInitA(&data.structure, 256);
    data.loadUserConfig = true;

    parseConfigFileSystem(&instance, &data);
    parseConfigFileUser(&instance, &data);
    parseArguments(&instance, &data, argc, argv);

    //If we don't have a custom structure, use the default one
    if(data.structure.length == 0)
        ffStrbufAppendS(&data.structure, FASTFETCH_DATATEXT_STRUCTURE);

    if(ffStrbufContainIgnCaseS(&data.structure, "CPUUsage"))
        ffPrepareCPUUsage();

    if(ffStrbufContainIgnCaseS(&data.structure, "PublicIp"))
        ffPreparePublicIp(&instance);

    if(ffStrbufContainIgnCaseS(&data.structure, "Weather"))
        ffPrepareWeather(&instance);

    ffStart(&instance);

    //Parse the structure and call the modules
    uint32_t startIndex = 0;
    while (startIndex < data.structure.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&data.structure, startIndex, ':');
        data.structure.chars[colonIndex] = '\0';

        parseStructureCommand(&instance, &data, data.structure.chars + startIndex);

        startIndex = colonIndex + 1;
    }

    ffFinish(&instance);

    ffStrbufDestroy(&data.structure);
    ffValuestoreDestroy(&data.customValues);

    ffDestroyInstance(&instance);
}
