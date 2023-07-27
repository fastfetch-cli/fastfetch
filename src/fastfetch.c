#include "fastfetch.h"
#include "common/printing.h"
#include "common/parsing.h"
#include "common/io/io.h"
#include "common/time.h"
#include "common/jsonconfig.h"
#include "util/stringUtils.h"
#include "logo/logo.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

#ifdef WIN32
    #include "util/windows/getline.h"
#endif

#include "modules/modules.h"

typedef struct FFCustomValue
{
    bool printKey;
    FFstrbuf key;
    FFstrbuf value;
} FFCustomValue;

// Things only needed by fastfetch
typedef struct FFdata
{
    FFstrbuf structure;
    FFlist customValues; // List of FFCustomValue
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
    else if(ffStrEqualsIgnCase(command, "c") || ffStrEqualsIgnCase(command, "color"))
        puts(FASTFETCH_DATATEXT_HELP_COLOR);
    else if(ffStrEqualsIgnCase(command, "format"))
        puts(FASTFETCH_DATATEXT_HELP_FORMAT);
    else if(ffStrEqualsIgnCase(command, "load-config") || ffStrEqualsIgnCase(command, "loadconfig") || ffStrEqualsIgnCase(command, "config"))
        puts(FASTFETCH_DATATEXT_HELP_CONFIG);
    else if(ffStrEqualsIgnCase(command, "os-format"))
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
    else if(ffStrEqualsIgnCase(command, "host-format"))
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
    else if(ffStrEqualsIgnCase(command, "bios-format"))
    {
        constructAndPrintCommandHelpFormat("bios", "{2} {3}", 4,
            "bios date",
            "bios release",
            "bios vendor",
            "bios version"
        );
    }
    else if(ffStrEqualsIgnCase(command, "board-format"))
    {
        constructAndPrintCommandHelpFormat("board", "{2} {3}", 3,
            "board name",
            "board vendor",
            "board version"
        );
    }
    else if(ffStrEqualsIgnCase(command, "chassis-format"))
    {
        constructAndPrintCommandHelpFormat("chassis", "{2} {3}", 4,
            "chassis type",
            "chassis vendor",
            "chassis version"
        );
    }
    else if(ffStrEqualsIgnCase(command, "kernel-format"))
    {
        constructAndPrintCommandHelpFormat("kernel", "{2}", 3,
            "Kernel sysname",
            "Kernel release",
            "Kernel version"
        );
    }
    else if(ffStrEqualsIgnCase(command, "uptime-format"))
    {
        constructAndPrintCommandHelpFormat("uptime", "{} days {} hours {} mins", 4,
            "Days",
            "Hours",
            "Minutes",
            "Seconds"
        );
    }
    else if(ffStrEqualsIgnCase(command, "processes-format"))
    {
        constructAndPrintCommandHelpFormat("processes", "{}", 1,
            "Count"
        );
    }
    else if(ffStrEqualsIgnCase(command, "packages-format"))
    {
        constructAndPrintCommandHelpFormat("packages", "{2} (pacman){?3}[{3}]{?}, {4} (dpkg), {5} (rpm), {6} (emerge), {7} (eopkg), {8} (xbps), {9} (nix-system), {10} (nix-user), {11} (nix-default), {12} (apk), {13} (pkg), {14} (flatpak-system), {15} (flatpack-user), {16} (snap), {17} (brew), {18} (brew-cask), {19} (port), {20} (scoop), {21} (choco), {22} (pkgtool), {23} (paludis)", 23,
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
            "Number of choco packages",
            "Number of pkgtool packages",
            "Number of paludis packages"
        );
    }
    else if(ffStrEqualsIgnCase(command, "shell-format"))
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
    else if(ffStrEqualsIgnCase(command, "display-format"))
    {
        constructAndPrintCommandHelpFormat("display", "{}x{} @ {}Hz", 7,
            "Screen width",
            "Screen height",
            "Screen refresh rate",
            "Screen scaled width",
            "Screen scaled height",
            "Screen name",
            "Screen type",
            "Screen rotation"
        );
    }
    else if(ffStrEqualsIgnCase(command, "de-format"))
    {
        constructAndPrintCommandHelpFormat("de", "{2} {3}", 3,
            "DE process name",
            "DE pretty name",
            "DE version"
        );
    }
    else if(ffStrEqualsIgnCase(command, "wm-format"))
    {
        constructAndPrintCommandHelpFormat("wm", "{2} ({3})", 3,
            "WM process name",
            "WM pretty name",
            "WM protocol name"
        );
    }
    else if(ffStrEqualsIgnCase(command, "wmtheme-format"))
    {
        constructAndPrintCommandHelpFormat("wmtheme", "{}", 1,
            "WM theme name"
        );
    }
    else if(ffStrEqualsIgnCase(command, "theme-format"))
    {
        constructAndPrintCommandHelpFormat("theme", "{}", 1,
            "Combined themes"
        );
    }
    else if(ffStrEqualsIgnCase(command, "icons-format"))
    {
        constructAndPrintCommandHelpFormat("icons", "{}", 1,
            "Combined icons"
        );
    }
    else if(ffStrEqualsIgnCase(command, "wallpaper-format"))
    {
        constructAndPrintCommandHelpFormat("wallpaper", "{}", 1,
            "Wallpaper image file"
        );
    }
    else if(ffStrEqualsIgnCase(command, "font-format"))
    {
        constructAndPrintCommandHelpFormat("font", "{} [QT], {} [GTK2], {} [GTK3], {} [GTK4]", 4,
            "Font 1",
            "Font 2",
            "Font 3",
            "Font 4"
        );
    }
    else if(ffStrEqualsIgnCase(command, "cursor-format"))
    {
        constructAndPrintCommandHelpFormat("cursor", "{} ({}pt)", 2,
            "Cursor theme",
            "Cursor size"
        );
    }
    else if(ffStrEqualsIgnCase(command, "terminal-format"))
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
    else if(ffStrEqualsIgnCase(command, "terminalfont-format"))
    {
        constructAndPrintCommandHelpFormat("terminalfont", "{}", 4,
            "Terminal font",
            "Terminal font name",
            "Termianl font size",
            "Terminal font styles"
        );
    }
    else if(ffStrEqualsIgnCase(command, "cpu-format"))
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
    else if(ffStrEqualsIgnCase(command, "cpu-usage-format"))
    {
        constructAndPrintCommandHelpFormat("cpu-usage", "{0}%", 1,
            "CPU usage without percent mark"
        );
    }
    else if(ffStrEqualsIgnCase(command, "gpu-format"))
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
    else if(ffStrEqualsIgnCase(command, "memory-format"))
    {
        constructAndPrintCommandHelpFormat("memory", "{} / {} ({}%)", 3,
            "Used size",
            "Total size",
            "Percentage used"
        );
    }
    else if(ffStrEqualsIgnCase(command, "swap-format"))
    {
        constructAndPrintCommandHelpFormat("swap", "{} / {} ({}%)", 3,
            "Used size",
            "Total size",
            "Percentage used"
        );
    }
    else if(ffStrEqualsIgnCase(command, "disk-format"))
    {
        constructAndPrintCommandHelpFormat("disk", "{1} / {2} ({3}%)", 9,
            "Size used",
            "Size total",
            "Size percentage",
            "Files used",
            "Files total",
            "Files percentage",
            "True if external volume",
            "True if hidden volume",
            "Filesystem"
        );
    }
    else if(ffStrEqualsIgnCase(command, "battery-format"))
    {
        constructAndPrintCommandHelpFormat("battery", "{}%, {}", 5,
            "Battery manufactor",
            "Battery model",
            "Battery technology",
            "Battery capacity",
            "Battery status"
        );
    }
    else if(ffStrEqualsIgnCase(command, "poweradapter-format"))
    {
        constructAndPrintCommandHelpFormat("poweradapter", "{}%, {}", 5,
            "PowerAdapter watts",
            "PowerAdapter name",
            "PowerAdapter manufacturer",
            "PowerAdapter model",
            "PowerAdapter description"
        );
    }
    else if(ffStrEqualsIgnCase(command, "lm-format"))
    {
        constructAndPrintCommandHelpFormat("lm", "{} ({})", 2,
            "LM service",
            "LM type"
        );
    }
    else if(ffStrEqualsIgnCase(command, "locale-format"))
    {
        constructAndPrintCommandHelpFormat("locale", "{}", 1,
            "Locale code"
        );
    }
    else if(ffStrEqualsIgnCase(command, "local-ip-format"))
    {
        constructAndPrintCommandHelpFormat("local-ip", "{}", 1,
            "Local IP address"
        );
    }
    else if(ffStrEqualsIgnCase(command, "public-ip-format"))
    {
        constructAndPrintCommandHelpFormat("public-ip", "{}", 1,
            "Public IP address"
        );
    }
    else if(ffStrEqualsIgnCase(command, "wifi-format"))
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
    else if(ffStrEqualsIgnCase(command, "player-format"))
    {
        constructAndPrintCommandHelpFormat("player", "{}", 4,
            "Pretty player name",
            "Player name",
            "Player Identifier",
            "URL name"
        );
    }
    else if(ffStrEqualsIgnCase(command, "media-format"))
    {
        constructAndPrintCommandHelpFormat("media", "{3} - {1}", 4,
            "Pretty media name",
            "Media name",
            "Artist name",
            "Album name"
        );
    }
    else if(ffStrEqualsIgnCase(command, "datetime-format") == 0 || ffStrEqualsIgnCase(command, "date-format") || ffStrEqualsIgnCase(command, "time-format"))
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
    else if(ffStrEqualsIgnCase(command, "vulkan-format"))
    {
        constructAndPrintCommandHelpFormat("vulkan", "{} (driver), {} (api version)", 3,
            "Driver name",
            "API version",
            "Conformance version"
        );
    }
    else if(ffStrEqualsIgnCase(command, "opengl-format"))
    {
        constructAndPrintCommandHelpFormat("opengl", "{}", 3,
            "version",
            "renderer",
            "vendor",
            "shading language version"
        );
    }
    else if(ffStrEqualsIgnCase(command, "opencl-format"))
    {
        constructAndPrintCommandHelpFormat("opencl", "{}", 3,
            "version",
            "device",
            "vendor"
        );
    }
    else if(ffStrEqualsIgnCase(command, "bluetooth-format"))
    {
        constructAndPrintCommandHelpFormat("bluetooth", "{1} (4%)", 4,
            "Name",
            "Address",
            "Type",
            "Battery percentage"
        );
    }
    else if(ffStrEqualsIgnCase(command, "sound-format"))
    {
        constructAndPrintCommandHelpFormat("sound", "{2} (3%)", 4,
            "Main",
            "Name",
            "Volume",
            "Identifier"
        );
    }
    else if(ffStrEqualsIgnCase(command, "gamepad-format"))
    {
        constructAndPrintCommandHelpFormat("gamepad", "{1}", 1,
            "Name",
            "Identifier"
        );
    }
    else
        fprintf(stderr, "No specific help for command %s provided\n", command);
}

static void listAvailablePresets(void)
{
    FF_LIST_FOR_EACH(FFstrbuf, path, instance.state.platform.dataDirs)
    {
        ffStrbufAppendS(path, "fastfetch/presets/");
        ffListFilesRecursively(path->chars);
    }
}

static void listAvailableLogos(void)
{
    FF_LIST_FOR_EACH(FFstrbuf, path, instance.state.platform.dataDirs)
    {
        ffStrbufAppendS(path, "fastfetch/logos/");
        ffListFilesRecursively(path->chars);
    }
}

static void listConfigPaths(void)
{
    FF_LIST_FOR_EACH(FFstrbuf, folder, instance.state.platform.configDirs)
    {
        bool exists = false;
        ffStrbufAppendS(folder, "fastfetch/config.jsonc");
        exists = ffPathExists(folder->chars, FF_PATHTYPE_FILE);
        if (!exists)
        {
            ffStrbufSubstrBefore(folder, folder->length - (uint32_t) strlen("jsonc"));
            ffStrbufAppendS(folder, "conf");
            exists = ffPathExists(folder->chars, FF_PATHTYPE_FILE);
        }
        printf("%s%s\n", folder->chars, exists ? " (*)" : "");
    }
}

static void listDataPaths(void)
{
    FF_LIST_FOR_EACH(FFstrbuf, folder, instance.state.platform.dataDirs)
    {
        ffStrbufAppendS(folder, "fastfetch/");
        puts(folder->chars);
    }
}

static void parseOption(FFdata* data, const char* key, const char* value);

static bool parseJsoncFile(const char* path)
{
    yyjson_read_err error;
    yyjson_doc* doc = yyjson_read_file(path, YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS | YYJSON_READ_ALLOW_INF_AND_NAN, NULL, &error);
    if (!doc)
    {
        if (error.code != YYJSON_READ_ERROR_FILE_OPEN)
        {
            fprintf(stderr, "ERROR: failed to parse JSON config file `%s` at pos %zu: %s\n", path, error.pos, error.msg);
            exit(477);
        }
        return false;
    }
    if (instance.state.configDoc)
        yyjson_doc_free(instance.state.configDoc); // for `--load-config`

    instance.state.configDoc = doc;
    return true;
}

static bool parseConfigFile(FFdata* data, const char* path)
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
            parseOption(data, lineStart, NULL);
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

        parseOption(data, lineStart, valueStart);
    }

    if(line != NULL)
        free(line);

    fclose(file);
    return true;
}

static void generateConfigFile(bool force)
{
    FFstrbuf* filename = (FFstrbuf*) ffListGet(&instance.state.platform.configDirs, 0);
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

static void optionParseConfigFile(FFdata* data, const char* key, const char* value)
{
    if(value == NULL)
    {
        fprintf(stderr, "Error: usage: %s <file>\n", key);
        exit(413);
    }
    uint32_t fileNameLen = (uint32_t) strlen(value);
    if(fileNameLen == 0)
    {
        fprintf(stderr, "Error: usage: %s <file>\n", key);
        exit(413);
    }

    bool isJsonConfig = fileNameLen > strlen(".jsonc") && strcasecmp(value + fileNameLen - strlen(".jsonc"), ".jsonc") == 0;

    //Try to load as an absolute path

    if(isJsonConfig ? parseJsoncFile(value) : parseConfigFile(data, value))
        return;

    //Try to load as a relative path

    FF_STRBUF_AUTO_DESTROY absolutePath = ffStrbufCreateA(128);

    FF_LIST_FOR_EACH(FFstrbuf, path, instance.state.platform.dataDirs)
    {
        //We need to copy it, because if a config file loads a config file, the value of path must be unchanged
        ffStrbufSet(&absolutePath, path);
        ffStrbufAppendS(&absolutePath, "fastfetch/presets/");
        ffStrbufAppendS(&absolutePath, value);

        bool success = isJsonConfig ? parseJsoncFile(value) : parseConfigFile(data, absolutePath.chars);

        if(success)
            return;
    }

    //File not found

    fprintf(stderr, "Error: couldn't find config: %s\n", value);
    exit(414);
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

static void parseOption(FFdata* data, const char* key, const char* value)
{
    ///////////////////////
    //Informative options//
    ///////////////////////

    if(ffStrEqualsIgnCase(key, "-h") || ffStrEqualsIgnCase(key, "--help"))
    {
        printCommandHelp(value);
        exit(0);
    }
    else if(ffStrEqualsIgnCase(key, "-v") || ffStrEqualsIgnCase(key, "--version"))
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
    else if(ffStrEqualsIgnCase(key, "--version-raw"))
    {
        puts(FASTFETCH_PROJECT_VERSION);
        exit(0);
    }
    else if(ffStrStartsWithIgnCase(key, "--print"))
    {
        const char* subkey = key + strlen("--print");
        if(ffStrEqualsIgnCase(subkey, "-config-system"))
        {
            puts(FASTFETCH_DATATEXT_CONFIG_SYSTEM);
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "-config-user"))
        {
            puts(FASTFETCH_DATATEXT_CONFIG_USER);
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "-structure"))
        {
            puts(FASTFETCH_DATATEXT_STRUCTURE);
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "-logos"))
        {
            ffLogoBuiltinPrint();
            exit(0);
        }
        else
            goto error;
    }
    else if(ffStrStartsWithIgnCase(key, "--list"))
    {
        const char* subkey = key + strlen("--list");
        if(ffStrEqualsIgnCase(subkey, "-modules"))
        {
            puts(FASTFETCH_DATATEXT_MODULES);
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "-presets"))
        {
            listAvailablePresets();
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "-config-paths"))
        {
            listConfigPaths();
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "-data-paths"))
        {
            listDataPaths();
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "-features"))
        {
            ffListFeatures();
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "-logos"))
        {
            puts("Builtin logos:");
            ffLogoBuiltinList();
            puts("\nCustom logos:");
            listAvailableLogos();
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "-logos-autocompletion"))
        {
            ffLogoBuiltinListAutocompletion();
            exit(0);
        }
        else
            goto error;
    }
    else if(ffStrStartsWithIgnCase(key, "--set"))
    {
        const char* subkey = key + strlen("--set");
        if(*subkey != '\0' && !ffStrEqualsIgnCase(subkey, "-keyless"))
            goto error;

        FF_STRBUF_AUTO_DESTROY customValueStr = ffStrbufCreate();
        ffOptionParseString(key, value, &customValueStr);
        uint32_t index = ffStrbufFirstIndexC(&customValueStr, '=');
        if(index == 0 || index == customValueStr.length)
        {
            fprintf(stderr, "Error: usage: %s <key>=<str>\n", key);
            exit(477);
        }
        FFCustomValue* customValue = (FFCustomValue*) ffListAdd(&data->customValues);
        ffStrbufInitS(&customValue->value, &customValueStr.chars[index + 1]);
        ffStrbufSubstrBefore(&customValueStr, index);
        ffStrbufInitMove(&customValue->key, &customValueStr);
        customValue->printKey = *subkey == '\0';
    }

    ///////////////////
    //General options//
    ///////////////////

    else if(ffStrEqualsIgnCase(key, "-r") || ffStrEqualsIgnCase(key, "--recache"))
        instance.config.recache = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--load-config"))
        optionParseConfigFile(data, key, value);
    else if(ffStrEqualsIgnCase(key, "--gen-config"))
        generateConfigFile(false);
    else if(ffStrEqualsIgnCase(key, "--gen-config-force"))
        generateConfigFile(true);
    else if(ffStrEqualsIgnCase(key, "--thread") || ffStrEqualsIgnCase(key, "--multithreading"))
        instance.config.multithreading = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--stat"))
    {
        if((instance.config.stat = ffOptionParseBoolean(value)))
            instance.config.showErrors = true;
    }
    else if(ffStrEqualsIgnCase(key, "--allow-slow-operations"))
        instance.config.allowSlowOperations = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--escape-bedrock"))
        instance.config.escapeBedrock = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--pipe"))
        instance.config.pipe = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--load-user-config"))
        data->loadUserConfig = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--processing-timeout"))
        instance.config.processingTimeout = ffOptionParseInt32(key, value);

    #if defined(__linux__) || defined(__FreeBSD__)
    else if(ffStrEqualsIgnCase(key, "--player-name"))
        ffOptionParseString(key, value, &instance.config.playerName);
    else if (ffStrEqualsIgnCase(key, "--os-file"))
        ffOptionParseString(key, value, &instance.config.osFile);
    #elif defined(_WIN32)
    else if (ffStrEqualsIgnCase(key, "--wmi-timeout"))
        instance.config.wmiTimeout = ffOptionParseInt32(key, value);
    #endif

    ////////////////
    //Logo options//
    ////////////////

    else if(ffParseLogoCommandOptions(&instance.config.logo, key, value)) {}

    ///////////////////
    //Display options//
    ///////////////////

    else if(ffStrEqualsIgnCase(key, "--show-errors"))
        instance.config.showErrors = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--disable-linewrap"))
        instance.config.disableLinewrap = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--hide-cursor"))
        instance.config.hideCursor = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "-s") || ffStrEqualsIgnCase(key, "--structure"))
        ffOptionParseString(key, value, &data->structure);
    else if(ffStrEqualsIgnCase(key, "--separator"))
        ffOptionParseString(key, value, &instance.config.keyValueSeparator);
    else if(ffStrEqualsIgnCase(key, "--color-keys"))
    {
        optionCheckString(key, value, &instance.config.colorKeys);
        ffOptionParseColor(value, &instance.config.colorKeys);
    }
    else if(ffStrEqualsIgnCase(key, "--color-title"))
    {
        optionCheckString(key, value, &instance.config.colorTitle);
        ffOptionParseColor(value, &instance.config.colorTitle);
    }
    else if(ffStrEqualsIgnCase(key, "-c") || ffStrEqualsIgnCase(key, "--color"))
    {
        optionCheckString(key, value, &instance.config.colorKeys);
        ffOptionParseColor(value, &instance.config.colorKeys);
        ffStrbufSet(&instance.config.colorTitle, &instance.config.colorKeys);
    }
    else if(ffStrEqualsIgnCase(key, "--binary-prefix"))
    {
        instance.config.binaryPrefixType = (FFBinaryPrefixType) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "iec", FF_BINARY_PREFIX_TYPE_IEC },
            { "si", FF_BINARY_PREFIX_TYPE_SI },
            { "jedec", FF_BINARY_PREFIX_TYPE_JEDEC },
            {}
        });
    }
    else if(ffStrEqualsIgnCase(key, "--size-ndigits"))
        instance.config.sizeNdigits = (uint8_t) ffOptionParseUInt32(key, value);
    else if(ffStrEqualsIgnCase(key, "--size-max-prefix"))
    {
        instance.config.sizeMaxPrefix = (uint8_t) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "B", 0 },
            { "kB", 1 },
            { "MB", 2 },
            { "GB", 3 },
            { "TB", 4 },
            { "PB", 5 },
            { "EB", 6 },
            { "ZB", 7 },
            { "YB", 8 },
            {}
        });
    }
    else if(ffStrEqualsIgnCase(key, "--temperature-unit"))
    {
        instance.config.temperatureUnit = (FFTemperatureUnit) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "CELSIUS", FF_TEMPERATURE_UNIT_CELSIUS },
            { "C", FF_TEMPERATURE_UNIT_CELSIUS },
            { "FAHRENHEIT", FF_TEMPERATURE_UNIT_FAHRENHEIT },
            { "F", FF_TEMPERATURE_UNIT_FAHRENHEIT },
            { "KELVIN", FF_TEMPERATURE_UNIT_KELVIN },
            { "K", FF_TEMPERATURE_UNIT_KELVIN },
            {},
        });
    }
    else if(ffStrEqualsIgnCase(key, "--percent-type"))
        instance.config.percentType = ffOptionParseUInt32(key, value);
    else if(ffStrEqualsIgnCase(key, "--no-buffer"))
        instance.config.noBuffer = ffOptionParseBoolean(value);

    ///////////////////////
    //Module args options//
    ///////////////////////

    else if(ffParseOSCommandOptions(&instance.config.os, key, value)) {}
    else if(ffParseHostCommandOptions(&instance.config.host, key, value)) {}
    else if(ffParseOSCommandOptions(&instance.config.os, key, value)) {}
    else if(ffParseBiosCommandOptions(&instance.config.bios, key, value)) {}
    else if(ffParseBoardCommandOptions(&instance.config.board, key, value)) {}
    else if(ffParseChassisCommandOptions(&instance.config.chassis, key, value)) {}
    else if(ffParseCommandCommandOptions(&instance.config.command, key, value)) {}
    else if(ffParseCustomCommandOptions(&instance.config.custom, key, value)) {}
    else if(ffParseKernelCommandOptions(&instance.config.kernel, key, value)) {}
    else if(ffParseUptimeCommandOptions(&instance.config.uptime, key, value)) {}
    else if(ffParseProcessesCommandOptions(&instance.config.processes, key, value)) {}
    else if(ffParsePackagesCommandOptions(&instance.config.packages, key, value)) {}
    else if(ffParseShellCommandOptions(&instance.config.shell, key, value)) {}
    else if(ffParseDisplayCommandOptions(&instance.config.display, key, value)) {}
    else if(ffParseBrightnessCommandOptions(&instance.config.brightness, key, value)) {}
    else if(ffParseDECommandOptions(&instance.config.de, key, value)) {}
    else if(ffParseWifiCommandOptions(&instance.config.wifi, key, value)) {}
    else if(ffParseWMCommandOptions(&instance.config.wm, key, value)) {}
    else if(ffParseWMThemeCommandOptions(&instance.config.wmTheme, key, value)) {}
    else if(ffParseTitleCommandOptions(&instance.config.title, key, value)) {}
    else if(ffParseThemeCommandOptions(&instance.config.theme, key, value)) {}
    else if(ffParseIconsCommandOptions(&instance.config.icons, key, value)) {}
    else if(ffParseWallpaperCommandOptions(&instance.config.wallpaper, key, value)) {}
    else if(ffParseFontCommandOptions(&instance.config.font, key, value)) {}
    else if(ffParseCursorCommandOptions(&instance.config.cursor, key, value)) {}
    else if(ffParseTerminalCommandOptions(&instance.config.terminal, key, value)) {}
    else if(ffParseTerminalFontCommandOptions(&instance.config.terminalFont, key, value)) {}
    else if(ffParseTerminalSizeCommandOptions(&instance.config.terminalSize, key, value)) {}
    else if(ffParseCPUCommandOptions(&instance.config.cpu, key, value)) {}
    else if(ffParseCPUUsageCommandOptions(&instance.config.cpuUsage, key, value)) {}
    else if(ffParseGPUCommandOptions(&instance.config.gpu, key, value)) {}
    else if(ffParseMemoryCommandOptions(&instance.config.memory, key, value)) {}
    else if(ffParseSwapCommandOptions(&instance.config.swap, key, value)) {}
    else if(ffParseDiskCommandOptions(&instance.config.disk, key, value)) {}
    else if(ffParseBatteryCommandOptions(&instance.config.battery, key, value)) {}
    else if(ffParsePowerAdapterCommandOptions(&instance.config.powerAdapter, key, value)) {}
    else if(ffParseLocaleCommandOptions(&instance.config.locale, key, value)) {}
    else if(ffParseLocalIpCommandOptions(&instance.config.localIP, key, value)) {}
    else if(ffParsePublicIpCommandOptions(&instance.config.publicIP, key, value)) {}
    else if(ffParseWeatherCommandOptions(&instance.config.weather, key, value)) {}
    else if(ffParsePlayerCommandOptions(&instance.config.player, key, value)) {}
    else if(ffParseMediaCommandOptions(&instance.config.media, key, value)) {}
    else if(ffParseDateTimeCommandOptions(&instance.config.dateTime, key, value)) {}
    else if(ffParseVulkanCommandOptions(&instance.config.vulkan, key, value)) {}
    else if(ffParseOpenGLCommandOptions(&instance.config.openGL, key, value)) {}
    else if(ffParseOpenCLCommandOptions(&instance.config.openCL, key, value)) {}
    else if(ffParseUsersCommandOptions(&instance.config.users, key, value)) {}
    else if(ffParseBluetoothCommandOptions(&instance.config.bluetooth, key, value)) {}
    else if(ffParseSeparatorCommandOptions(&instance.config.separator, key, value)) {}
    else if(ffParseSoundCommandOptions(&instance.config.sound, key, value)) {}
    else if(ffParseGamepadCommandOptions(&instance.config.gamepad, key, value)) {}
    else if(ffParseColorsCommandOptions(&instance.config.colors, key, value)) {}

    ///////////////////
    //Library options//
    ///////////////////

    else if(ffStrStartsWithIgnCase(key, "--lib"))
    {
        const char* subkey = key + strlen("--lib");
        if(ffStrEqualsIgnCase(subkey, "-PCI"))
            ffOptionParseString(key, value, &instance.config.libPCI);
        else if(ffStrEqualsIgnCase(subkey, "-vulkan"))
            ffOptionParseString(key, value, &instance.config.libVulkan);
        else if(ffStrEqualsIgnCase(subkey, "-freetype"))
            ffOptionParseString(key, value, &instance.config.libfreetype);
        else if(ffStrEqualsIgnCase(subkey, "-wayland"))
            ffOptionParseString(key, value, &instance.config.libWayland);
        else if(ffStrEqualsIgnCase(subkey, "-xcb-randr"))
            ffOptionParseString(key, value, &instance.config.libXcbRandr);
        else if(ffStrEqualsIgnCase(subkey, "-xcb"))
            ffOptionParseString(key, value, &instance.config.libXcb);
        else if(ffStrEqualsIgnCase(subkey, "-Xrandr"))
            ffOptionParseString(key, value, &instance.config.libXrandr);
        else if(ffStrEqualsIgnCase(subkey, "-X11"))
            ffOptionParseString(key, value, &instance.config.libX11);
        else if(ffStrEqualsIgnCase(subkey, "-gio"))
            ffOptionParseString(key, value, &instance.config.libGIO);
        else if(ffStrEqualsIgnCase(subkey, "-DConf"))
            ffOptionParseString(key, value, &instance.config.libDConf);
        else if(ffStrEqualsIgnCase(subkey, "-dbus"))
            ffOptionParseString(key, value, &instance.config.libDBus);
        else if(ffStrEqualsIgnCase(subkey, "-XFConf"))
            ffOptionParseString(key, value, &instance.config.libXFConf);
        else if(ffStrEqualsIgnCase(subkey, "-sqlite") || ffStrEqualsIgnCase(subkey, "-sqlite3"))
            ffOptionParseString(key, value, &instance.config.libSQLite3);
        else if(ffStrEqualsIgnCase(subkey, "-rpm"))
            ffOptionParseString(key, value, &instance.config.librpm);
        else if(ffStrEqualsIgnCase(subkey, "-imagemagick"))
            ffOptionParseString(key, value, &instance.config.libImageMagick);
        else if(ffStrEqualsIgnCase(subkey, "-z"))
            ffOptionParseString(key, value, &instance.config.libZ);
        else if(ffStrEqualsIgnCase(subkey, "-chafa"))
            ffOptionParseString(key, value, &instance.config.libChafa);
        else if(ffStrEqualsIgnCase(subkey, "-egl"))
            ffOptionParseString(key, value, &instance.config.libEGL);
        else if(ffStrEqualsIgnCase(subkey, "-glx"))
            ffOptionParseString(key, value, &instance.config.libGLX);
        else if(ffStrEqualsIgnCase(subkey, "-osmesa"))
            ffOptionParseString(key, value, &instance.config.libOSMesa);
        else if(ffStrEqualsIgnCase(subkey, "-opencl"))
            ffOptionParseString(key, value, &instance.config.libOpenCL);
        else if(ffStrEqualsIgnCase(key, "-pulse"))
            ffOptionParseString(key, value, &instance.config.libPulse);
        else if(ffStrEqualsIgnCase(subkey, "-nm"))
            ffOptionParseString(key, value, &instance.config.libnm);
        else if(ffStrEqualsIgnCase(subkey, "-ddcutil"))
            ffOptionParseString(key, value, &instance.config.libDdcutil);
        else
            goto error;
    }

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

static void parseConfigFiles(FFdata* data)
{
    for(uint32_t i = instance.state.platform.configDirs.length; i > 0; --i)
    {
        FFstrbuf* dir = ffListGet(&instance.state.platform.configDirs, i - 1);
        uint32_t dirLength = dir->length;

        ffStrbufAppendS(dir, "fastfetch/config.jsonc");
        bool success = parseJsoncFile(dir->chars);
        ffStrbufSubstrBefore(dir, dirLength);
        if (success) return;
    }

    for(uint32_t i = instance.state.platform.configDirs.length; i > 0; --i)
    {
        if(!data->loadUserConfig)
            return;

        FFstrbuf* dir = ffListGet(&instance.state.platform.configDirs, i - 1);
        uint32_t dirLength = dir->length;

        ffStrbufAppendS(dir, "fastfetch/config.conf");
        parseConfigFile(data, dir->chars);
        ffStrbufSubstrBefore(dir, dirLength);
    }
}

static void parseArguments(FFdata* data, int argc, const char** argv)
{
    if(!data->loadUserConfig)
        return;

    for(int i = 1; i < argc; i++)
    {
        if(i == argc - 1 || (
            *argv[i + 1] == '-' &&
            strcasecmp(argv[i], "--separator-string") != 0 // Separator string can start with a -
        )) {
            parseOption(data, argv[i], NULL);
        }
        else
        {
            parseOption(data, argv[i], argv[i + 1]);
            ++i;
        }
    }
}

static void parseStructureCommand(const char* line, FFlist* customValues)
{
    // handle `--set` and `--set-keyless`
    FF_LIST_FOR_EACH(FFCustomValue, customValue, *customValues)
    {
        if (ffStrbufEqualS(&customValue->key, line))
        {
            __attribute__((__cleanup__(ffDestroyCustomOptions))) FFCustomOptions options;
            ffInitCustomOptions(&options);
            if (customValue->printKey)
                ffStrbufAppend(&options.moduleArgs.key, &customValue->key);
            ffStrbufAppend(&options.moduleArgs.outputFormat, &customValue->value);
            ffPrintCustom(&options);
            return;
        }
    }

    if(ffStrEqualsIgnCase(line, FF_BREAK_MODULE_NAME))
        ffPrintBreak();
    else if(ffStrEqualsIgnCase(line, FF_TITLE_MODULE_NAME))
        ffPrintTitle(&instance.config.title);
    else if(ffStrEqualsIgnCase(line, FF_SEPARATOR_MODULE_NAME))
        ffPrintSeparator(&instance.config.separator);
    else if(ffStrEqualsIgnCase(line, FF_OS_MODULE_NAME))
        ffPrintOS(&instance.config.os);
    else if(ffStrEqualsIgnCase(line, FF_HOST_MODULE_NAME))
        ffPrintHost(&instance.config.host);
    else if(ffStrEqualsIgnCase(line, FF_BIOS_MODULE_NAME))
        ffPrintBios(&instance.config.bios);
    else if(ffStrEqualsIgnCase(line, FF_BOARD_MODULE_NAME))
        ffPrintBoard(&instance.config.board);
    else if(ffStrEqualsIgnCase(line, FF_BRIGHTNESS_MODULE_NAME))
        ffPrintBrightness(&instance.config.brightness);
    else if(ffStrEqualsIgnCase(line, FF_CHASSIS_MODULE_NAME))
        ffPrintChassis(&instance.config.chassis);
    else if(ffStrEqualsIgnCase(line, FF_KERNEL_MODULE_NAME))
        ffPrintKernel(&instance.config.kernel);
    else if(ffStrEqualsIgnCase(line, FF_PROCESSES_MODULE_NAME))
        ffPrintProcesses(&instance.config.processes);
    else if(ffStrEqualsIgnCase(line, FF_UPTIME_MODULE_NAME))
        ffPrintUptime(&instance.config.uptime);
    else if(ffStrEqualsIgnCase(line, FF_PACKAGES_MODULE_NAME))
        ffPrintPackages(&instance.config.packages);
    else if(ffStrEqualsIgnCase(line, FF_SHELL_MODULE_NAME))
        ffPrintShell(&instance.config.shell);
    else if(ffStrEqualsIgnCase(line, FF_DISPLAY_MODULE_NAME))
        ffPrintDisplay(&instance.config.display);
    else if(ffStrEqualsIgnCase(line, FF_DE_MODULE_NAME))
        ffPrintDE(&instance.config.de);
    else if(ffStrEqualsIgnCase(line, FF_WM_MODULE_NAME))
        ffPrintWM(&instance.config.wm);
    else if(ffStrEqualsIgnCase(line, FF_THEME_MODULE_NAME))
        ffPrintTheme(&instance.config.theme);
    else if(ffStrEqualsIgnCase(line, FF_WMTHEME_MODULE_NAME))
        ffPrintWMTheme(&instance.config.wmTheme);
    else if(ffStrEqualsIgnCase(line, FF_ICONS_MODULE_NAME))
        ffPrintIcons(&instance.config.icons);
    else if(ffStrEqualsIgnCase(line, FF_WALLPAPER_MODULE_NAME))
        ffPrintWallpaper(&instance.config.wallpaper);
    else if(ffStrEqualsIgnCase(line, FF_FONT_MODULE_NAME))
        ffPrintFont(&instance.config.font);
    else if(ffStrEqualsIgnCase(line, FF_CURSOR_MODULE_NAME))
        ffPrintCursor(&instance.config.cursor);
    else if(ffStrEqualsIgnCase(line, FF_TERMINAL_MODULE_NAME))
        ffPrintTerminal(&instance.config.terminal);
    else if(ffStrEqualsIgnCase(line, FF_TERMINALFONT_MODULE_NAME))
        ffPrintTerminalFont(&instance.config.terminalFont);
    else if(ffStrEqualsIgnCase(line, FF_TERMINALSIZE_MODULE_NAME))
        ffPrintTerminalSize(&instance.config.terminalSize);
    else if(ffStrEqualsIgnCase(line, FF_CPU_MODULE_NAME))
        ffPrintCPU(&instance.config.cpu);
    else if(ffStrEqualsIgnCase(line, FF_CPUUSAGE_MODULE_NAME))
        ffPrintCPUUsage(&instance.config.cpuUsage);
    else if(ffStrEqualsIgnCase(line, FF_CUSTOM_MODULE_NAME))
        ffPrintCustom(&instance.config.custom);
    else if(ffStrEqualsIgnCase(line, FF_GPU_MODULE_NAME))
        ffPrintGPU(&instance.config.gpu);
    else if(ffStrEqualsIgnCase(line, FF_MEMORY_MODULE_NAME))
        ffPrintMemory(&instance.config.memory);
    else if(ffStrEqualsIgnCase(line, FF_SWAP_MODULE_NAME))
        ffPrintSwap(&instance.config.swap);
    else if(ffStrEqualsIgnCase(line, FF_DISK_MODULE_NAME))
        ffPrintDisk(&instance.config.disk);
    else if(ffStrEqualsIgnCase(line, FF_BATTERY_MODULE_NAME))
        ffPrintBattery(&instance.config.battery);
    else if(ffStrEqualsIgnCase(line, FF_POWERADAPTER_MODULE_NAME))
        ffPrintPowerAdapter(&instance.config.powerAdapter);
    else if(ffStrEqualsIgnCase(line, FF_LM_MODULE_NAME))
        ffPrintLM(&instance.config.lm);
    else if(ffStrEqualsIgnCase(line, FF_LOCALE_MODULE_NAME))
        ffPrintLocale(&instance.config.locale);
    else if(ffStrEqualsIgnCase(line, FF_LOCALIP_MODULE_NAME))
        ffPrintLocalIp(&instance.config.localIP);
    else if(ffStrEqualsIgnCase(line, FF_PUBLICIP_MODULE_NAME))
        ffPrintPublicIp(&instance.config.publicIP);
    else if(ffStrEqualsIgnCase(line, FF_WIFI_MODULE_NAME))
        ffPrintWifi(&instance.config.wifi);
    else if(ffStrEqualsIgnCase(line, FF_WEATHER_MODULE_NAME))
        ffPrintWeather(&instance.config.weather);
    else if(ffStrEqualsIgnCase(line, FF_PLAYER_MODULE_NAME))
        ffPrintPlayer(&instance.config.player);
    else if(ffStrEqualsIgnCase(line, FF_MEDIA_MODULE_NAME))
        ffPrintMedia(&instance.config.media);
    else if(ffStrEqualsIgnCase(line, FF_DATETIME_MODULE_NAME))
        ffPrintDateTime(&instance.config.dateTime);
    else if(ffStrEqualsIgnCase(line, FF_COLORS_MODULE_NAME))
        ffPrintColors(&instance.config.colors);
    else if(ffStrEqualsIgnCase(line, FF_VULKAN_MODULE_NAME))
        ffPrintVulkan(&instance.config.vulkan);
    else if(ffStrEqualsIgnCase(line, FF_OPENGL_MODULE_NAME))
        ffPrintOpenGL(&instance.config.openGL);
    else if(ffStrEqualsIgnCase(line, FF_OPENCL_MODULE_NAME))
        ffPrintOpenCL(&instance.config.openCL);
    else if(ffStrEqualsIgnCase(line, FF_USERS_MODULE_NAME))
        ffPrintUsers(&instance.config.users);
    else if(ffStrEqualsIgnCase(line, FF_COMMAND_MODULE_NAME))
        ffPrintCommand(&instance.config.command);
    else if(ffStrEqualsIgnCase(line, FF_BLUETOOTH_MODULE_NAME))
        ffPrintBluetooth(&instance.config.bluetooth);
    else if(ffStrEqualsIgnCase(line, FF_SOUND_MODULE_NAME))
        ffPrintSound(&instance.config.sound);
    else if(ffStrEqualsIgnCase(line, FF_GAMEPAD_MODULE_NAME))
        ffPrintGamepad(&instance.config.gamepad);
    else
        ffPrintErrorString(line, 0, NULL, NULL, "<no implementation provided>");
}

int main(int argc, const char** argv)
{
    ffInitInstance();

    //Data stores things only needed for the configuration of fastfetch
    FFdata data;
    ffStrbufInit(&data.structure);
    ffListInit(&data.customValues, sizeof(FFCustomValue));
    data.loadUserConfig = true;

    if(!getenv("NO_CONFIG"))
        parseConfigFiles(&data);
    parseArguments(&data, argc, argv);

    if (instance.state.configDoc)
    {
        const char* error = NULL;

        if (
            (error = ffParseLogoJsonConfig()) ||
            (error = ffParseGeneralJsonConfig()) ||
            (error = ffParseDisplayJsonConfig()) ||
            (error = ffParseLibraryJsonConfig()) ||
            false
        ) {
            fputs(error, stderr);
            exit(477);
        }
    }

    if(data.structure.length > 0 || !instance.state.configDoc)
    {
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
    }

    ffStart();

    #if defined(_WIN32)
        if (!instance.config.noBuffer) fflush(stdout);
    #endif

    if (data.structure.length == 0 && instance.state.configDoc)
    {
        ffPrintJsonConfig();
    }
    else
    {
        //Parse the structure and call the modules
        uint32_t startIndex = 0;
        while (startIndex < data.structure.length)
        {
            uint32_t colonIndex = ffStrbufNextIndexC(&data.structure, startIndex, ':');
            data.structure.chars[colonIndex] = '\0';

            uint64_t ms = 0;
            if(__builtin_expect(instance.config.stat, false))
                ms = ffTimeGetTick();

            parseStructureCommand(data.structure.chars + startIndex, &data.customValues);

            if(__builtin_expect(instance.config.stat, false))
            {
                char str[32];
                int len = snprintf(str, sizeof str, "%" PRIu64 "ms", ffTimeGetTick() - ms);
                if(instance.config.pipe)
                    puts(str);
                else
                    printf("\033[s\033[1A\033[9999999C\033[%dD%s\033[u", len, str); // Save; Up 1; Right 9999999; Left <len>; Print <str>; Load
            }

            #if defined(_WIN32)
                if (!instance.config.noBuffer) fflush(stdout);
            #endif

            startIndex = colonIndex + 1;
        }
    }

    ffFinish();

    ffStrbufDestroy(&data.structure);
    FF_LIST_FOR_EACH(FFCustomValue, customValue, data.customValues)
    {
        ffStrbufDestroy(&customValue->key);
        ffStrbufDestroy(&customValue->value);
    }
    ffListDestroy(&data.customValues);

    ffDestroyInstance();
}
