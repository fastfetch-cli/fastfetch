#include "fastfetch.h"
#include "common/commandoption.h"
#include "common/printing.h"
#include "common/parsing.h"
#include "common/io/io.h"
#include "common/time.h"
#include "common/jsonconfig.h"
#include "detection/version/version.h"
#include "util/stringUtils.h"
#include "logo/logo.h"
#include "fastfetch_datatext.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>

#ifdef WIN32
    #include "util/windows/getline.h"
#endif

#include "modules/modules.h"

static void printCommandFormatHelp(const char* command)
{
    FF_STRBUF_AUTO_DESTROY type = ffStrbufCreateNS((uint32_t) (strlen(command) - strlen("-format")), command);
    for (FFModuleBaseInfo** modules = ffModuleInfos[toupper(command[0]) - 'A']; *modules; ++modules)
    {
        FFModuleBaseInfo* baseInfo = *modules;
        if (ffStrbufIgnCaseEqualS(&type, baseInfo->name))
        {
            if (baseInfo->printHelpFormat)
                baseInfo->printHelpFormat();
            else
                fprintf(stderr, "Error: Module '%s' doesn't support output formatting\n", baseInfo->name);
            return;
        }
    }

    fprintf(stderr, "Error: Module '%s' is not supported\n", type.chars);
}

static void printCommandHelp(const char* command)
{
    if(command == NULL)
    {
        static char input[] = FASTFETCH_DATATEXT_HELP;
        if (!instance.config.display.pipe)
        {
            char* token = strtok(input, "\n");
            puts(token);

            while ((token = strtok(NULL, "\n")))
            {
                // handle empty lines
                char* back = token;
                while (*--back != '\0');
                ffPrintCharTimes('\n', (uint32_t) (token - back - 1));

                if (isalpha(*token)) // highlight subjects
                {
                    char* colon = strchr(token, ':');
                    if (colon)
                    {
                        fputs("\e[1;4m", stdout); // Bold + Underline
                        fwrite(token, 1, (uint32_t) (colon - token), stdout);
                        fputs("\e[m", stdout);

                        if (colon - token == 5 && memcmp(token, "Usage", 5) == 0)
                        {
                            char* cmd = strstr(colon, "fastfetch ");
                            if (cmd)
                            {
                                fwrite(colon, 1, (uint32_t) (cmd - colon), stdout);
                                fputs("\e[1mfastfetch \e[m", stdout);
                                cmd += strlen("fastfetch ");
                                if (*cmd == '<')
                                {
                                    char* gt = strchr(cmd, '>');
                                    if (gt)
                                    {
                                        fputs("\e[3m", stdout);
                                        fwrite(cmd, 1, (uint32_t) (gt - cmd + 1), stdout);
                                        fputs("\e[m", stdout);
                                        cmd = gt + 1;
                                    }
                                }
                                puts(cmd);
                                continue;
                            }
                        }
                        puts(colon);
                        continue;
                    }
                }
                else if (*token == ' ') // highlight options. All options are indented
                {
                    char* dash = strchr(token, '-'); // start of an option
                    if (dash)
                    {
                        char* colon = strchr(dash, ':'); // end of an option
                        if (colon)
                        {
                            *colon = '\0';
                            char* lt = strrchr(dash, '<');
                            *colon = ':';
                            fputs("\e[1m", stdout); // Bold
                            fwrite(token, 1, (uint32_t) ((lt ? lt : colon) - token), stdout);
                            fputs("\e[m", stdout);
                            if (lt)
                            {
                                fputs("\e[3m", stdout);
                                fwrite(lt, 1, (uint32_t) (colon - lt), stdout);
                                fputs("\e[m", stdout);
                            }
                            puts(colon);
                            continue;
                        }
                    }
                }

                puts(token);
            }
        }
        else
        {
            puts(input);
        }
    }
    else if(ffStrEqualsIgnCase(command, "color"))
        puts(FASTFETCH_DATATEXT_HELP_COLOR);
    else if(ffStrEqualsIgnCase(command, "format"))
        puts(FASTFETCH_DATATEXT_HELP_FORMAT);
    else if(ffStrEqualsIgnCase(command, "load-config") || ffStrEqualsIgnCase(command, "config"))
        puts(FASTFETCH_DATATEXT_HELP_CONFIG);
    else if(isalpha(command[0]) && ffStrEndsWithIgnCase(command, "-format")) // x-format
        printCommandFormatHelp(command);
    else
        fprintf(stderr, "Error: No specific help for command '%s' provided\n", command);
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

static void listModules()
{
    unsigned count = 0;
    for (int i = 0; i <= 'Z' - 'A'; ++i)
    {
        for (FFModuleBaseInfo** modules = ffModuleInfos[i]; *modules; ++modules)
        {
            ++count;
            printf("%d)%s%s\n", count, count > 9 ? " " : "  ", (*modules)->name);
        }
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
            fprintf(stderr, "Error: failed to parse JSON config file `%s` at pos %zu: %s\n", path, error.pos, error.msg);
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
    FF_STRBUF_AUTO_DESTROY unescaped = ffStrbufCreate();

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

        if (strchr(valueStart, '\\'))
        {
            // Unescape all `\x`s
            const char* value = valueStart;
            while(*value != '\0')
            {
                if(*value != '\\')
                {
                    ffStrbufAppendC(&unescaped, *value);
                    ++value;
                    continue;
                }

                ++value;

                switch(*value)
                {
                    case 'n': ffStrbufAppendC(&unescaped, '\n'); break;
                    case 't': ffStrbufAppendC(&unescaped, '\t'); break;
                    case 'e': ffStrbufAppendC(&unescaped, '\e'); break;
                    case '\\': ffStrbufAppendC(&unescaped, '\\'); break;
                    default:
                        ffStrbufAppendC(&unescaped, '\\');
                        ffStrbufAppendC(&unescaped, *value);
                        break;
                }

                ++value;
            }
            parseOption(data, lineStart, unescaped.chars);
            ffStrbufClear(&unescaped);
        }
        else
        {
            parseOption(data, lineStart, valueStart);
        }
    }

    if(line != NULL)
        free(line);

    fclose(file);
    return true;
}

static void generateConfigFile(bool force, const char* type)
{
    if (!type)
    {
        fputs("Error: the config type (`jsonc` or `conf`) must be specified\n", stderr);
        exit(1);
    }

    FFstrbuf* filename = (FFstrbuf*) ffListGet(&instance.state.platform.configDirs, 0);
    // Paths generated in `init.c/initConfigDirs` end with `/`
    bool isJsonc = ffStrEqualsIgnCase(type, "jsonc");
    if (!isJsonc)
    {
        if (!ffStrEqualsIgnCase(type, "conf"))
        {
            fputs("Error: config type can only be `jsonc` or `conf`\n", stderr);
            exit(1);
        }
        else
            fputs("Warning: support of flag based config type `conf` is deprecated, and may be removed in the future\n", stderr);
    }

    ffStrbufAppendS(filename, isJsonc ? "fastfetch/config.jsonc" : "fastfetch/config.conf");

    if (!force && ffPathExists(filename->chars, FF_PATHTYPE_FILE))
    {
        fprintf(stderr, "Config file exists in `%s`, use `--gen-config-force` to overwrite\n", filename->chars);
        exit(1);
    }
    else
    {
        ffWriteFileData(
            filename->chars,
            isJsonc ? strlen(FASTFETCH_DATATEXT_CONFIG_USER_JSONC) : strlen(FASTFETCH_DATATEXT_CONFIG_USER),
            isJsonc ? FASTFETCH_DATATEXT_CONFIG_USER_JSONC : FASTFETCH_DATATEXT_CONFIG_USER);
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

    FF_STRBUF_AUTO_DESTROY absolutePath = ffStrbufCreateA(128);
    if (ffPathExpandEnv(value, &absolutePath))
    {
        bool success = isJsonConfig ? parseJsoncFile(absolutePath.chars) : parseConfigFile(data, absolutePath.chars);

        if(success)
            return;
    }

    //Try to load as a relative path

    FF_LIST_FOR_EACH(FFstrbuf, path, instance.state.platform.dataDirs)
    {
        //We need to copy it, because if a config file loads a config file, the value of path must be unchanged
        ffStrbufSet(&absolutePath, path);
        ffStrbufAppendS(&absolutePath, "fastfetch/presets/");
        ffStrbufAppendS(&absolutePath, value);

        bool success = isJsonConfig ? parseJsoncFile(absolutePath.chars) : parseConfigFile(data, absolutePath.chars);

        if(success)
            return;
    }

    {
        //Try exe path
        ffStrbufSet(&absolutePath, &instance.state.platform.exePath);
        ffStrbufSubstrBeforeLastC(&absolutePath, '/');
        ffStrbufAppendS(&absolutePath, "/presets/");
        ffStrbufAppendS(&absolutePath, value);

        bool success = isJsonConfig ? parseJsoncFile(absolutePath.chars) : parseConfigFile(data, absolutePath.chars);

        if(success)
            return;
    }

    //File not found

    fprintf(stderr, "Error: couldn't find config: %s\n", value);
    exit(414);
}

static void printVersion()
{
    FFVersionResult result = {};
    ffDetectVersion(&result);
    printf("%s %s%s%s (%s)\n", result.projectName, result.version, result.versionTweak, result.debugMode ? "-debug" : "", result.architecture);
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
        printVersion();
        exit(0);
    }
    else if(ffStrEqualsIgnCase(key, "--version-raw"))
    {
        puts(FASTFETCH_PROJECT_VERSION);
        exit(0);
    }
    else if(ffStrStartsWithIgnCase(key, "--print-"))
    {
        const char* subkey = key + strlen("--print-");
        if(ffStrEqualsIgnCase(subkey, "config-system"))
        {
            puts(FASTFETCH_DATATEXT_CONFIG_SYSTEM);
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "config-user"))
        {
            puts(FASTFETCH_DATATEXT_CONFIG_USER);
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "structure"))
        {
            puts(FASTFETCH_DATATEXT_STRUCTURE);
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "logos"))
        {
            ffLogoBuiltinPrint();
            exit(0);
        }
        else
            goto error;
    }
    else if(ffStrStartsWithIgnCase(key, "--list-"))
    {
        const char* subkey = key + strlen("--list-");
        if(ffStrEqualsIgnCase(subkey, "modules"))
        {
            listModules();
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "presets"))
        {
            listAvailablePresets();
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "config-paths"))
        {
            listConfigPaths();
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "data-paths"))
        {
            listDataPaths();
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "features"))
        {
            ffListFeatures();
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "logos"))
        {
            puts("Builtin logos:");
            ffLogoBuiltinList();
            puts("\nCustom logos:");
            listAvailableLogos();
            exit(0);
        }
        else if(ffStrEqualsIgnCase(subkey, "logos-autocompletion"))
        {
            ffLogoBuiltinListAutocompletion();
            exit(0);
        }
        else
            goto error;
    }
    else if(ffStrEqualsIgnCase(key, "--set") || ffStrEqualsIgnCase(key, "--set-keyless"))
    {
        FF_STRBUF_AUTO_DESTROY customValueStr = ffStrbufCreate();
        ffOptionParseString(key, value, &customValueStr);
        uint32_t index = ffStrbufFirstIndexC(&customValueStr, '=');
        if(index == 0 || index == customValueStr.length)
        {
            fprintf(stderr, "Error: usage: %s <key>=<str>\n", key);
            exit(477);
        }

        FF_STRBUF_AUTO_DESTROY customKey = ffStrbufCreateNS(index, customValueStr.chars);

        FFCustomValue* customValue = NULL;
        FF_LIST_FOR_EACH(FFCustomValue, x, data->customValues)
        {
            if(ffStrbufEqual(&x->key, &customKey))
            {
                ffStrbufDestroy(&x->key);
                ffStrbufDestroy(&x->value);
                customValue = x;
                break;
            }
        }
        if(!customValue) customValue = (FFCustomValue*) ffListAdd(&data->customValues);
        ffStrbufInitMove(&customValue->key, &customKey);
        ffStrbufSubstrAfter(&customValueStr, index);
        ffStrbufInitMove(&customValue->value, &customValueStr);
        customValue->printKey = key[5] == '\0';
    }

    ////////////
    //Switches//
    ////////////

    else if(ffStrEqualsIgnCase(key, "-c") || ffStrEqualsIgnCase(key, "--load-config") || ffStrEqualsIgnCase(key, "--config"))
        optionParseConfigFile(data, key, value);
    else if(ffStrEqualsIgnCase(key, "--gen-config"))
        generateConfigFile(false, value);
    else if(ffStrEqualsIgnCase(key, "--gen-config-force"))
        generateConfigFile(true, value);
    else if (ffStrEqualsIgnCase(key, "--migrate-config"))
    {
        if (instance.state.configDoc)
        {
            fputs("Error: existing jsonc config detected. Aborting\n", stderr);
            exit(477);
        }

        if (!value)
        {
            ffStrbufSet(&instance.state.migrateConfigPath, (FFstrbuf*) ffListGet(&instance.state.platform.configDirs, 0));
            ffStrbufAppendS(&instance.state.migrateConfigPath, "fastfetch/config.jsonc");
        }
        else
            ffStrbufSetS(&instance.state.migrateConfigPath, value);

        if (ffPathExists(instance.state.migrateConfigPath.chars, FF_PATHTYPE_ANY))
        {
            fprintf(stderr, "Error: file `%s` exists. Aborting\n", instance.state.migrateConfigPath.chars);
            exit(477);
        }
    }
    else if(ffStrEqualsIgnCase(key, "--load-user-config"))
        data->loadUserConfig = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--format"))
    {
        switch (ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "default", 0},
            { "json", 1 },
            {},
        }))
        {
            case 0:
                if (instance.state.resultDoc)
                {
                    yyjson_mut_doc_free(instance.state.resultDoc);
                    instance.state.resultDoc = NULL;
                }
                break;
            case 1:
                if (!instance.state.resultDoc)
                {
                    instance.state.resultDoc = yyjson_mut_doc_new(NULL);
                    yyjson_mut_doc_set_root(instance.state.resultDoc, yyjson_mut_arr(instance.state.resultDoc));
                }
                break;
        }
    }
    else if(ffStrEqualsIgnCase(key, "-s") || ffStrEqualsIgnCase(key, "--structure"))
        ffOptionParseString(key, value, &data->structure);

    ///////////
    //Options//
    ///////////

    else if(
        ffOptionsParseGeneralCommandLine(&instance.config.general, key, value) ||
        ffOptionsParseLogoCommandLine(&instance.config.logo, key, value) ||
        ffOptionsParseDisplayCommandLine(&instance.config.display, key, value) ||
        ffOptionsParseLibraryCommandLine(&instance.config.library, key, value) ||
        ffParseModuleOptions(key, value)
    ) {}

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
    if (__builtin_expect(instance.state.migrateConfigPath.length == 0, true))
    {
        for (uint32_t i = instance.state.platform.configDirs.length; i > 0; --i)
        {
            FFstrbuf* dir = ffListGet(&instance.state.platform.configDirs, i - 1);
            uint32_t dirLength = dir->length;

            ffStrbufAppendS(dir, "fastfetch/config.jsonc");
            bool success = parseJsoncFile(dir->chars);
            ffStrbufSubstrBefore(dir, dirLength);
            if (success) return;
        }
    }
    for (uint32_t i = instance.state.platform.configDirs.length; i > 0; --i)
    {
        if (!data->loadUserConfig)
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
        const char* key = argv[i];
        if(*key != '-')
        {
            fprintf(stderr, "Error: invalid option: %s. An option must start with `-`\n", key);
            exit(400);
        }

        if(i == argc - 1 || (
            argv[i + 1][0] == '-' &&
            argv[i + 1][1] != '\0' && // `-` is used as an alias for `/dev/stdin`
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

static void run(FFdata* data)
{
    if (instance.state.configDoc)
    {
        const char* error = NULL;

        yyjson_val* const root = yyjson_doc_get_root(instance.state.configDoc);
        if (!yyjson_is_obj(root))
            error = "Invalid JSON config format. Root value must be an object";

        if (
            error ||
            (error = ffOptionsParseLogoJsonConfig(&instance.config.logo, root)) ||
            (error = ffOptionsParseGeneralJsonConfig(&instance.config.general, root)) ||
            (error = ffOptionsParseDisplayJsonConfig(&instance.config.display, root)) ||
            (error = ffOptionsParseLibraryJsonConfig(&instance.config.library, root)) ||
            false
        ) {
            fprintf(stderr, "JsonConfig Error: %s\n", error);
            exit(477);
        }
    }

    const bool useJsonConfig = data->structure.length == 0 && instance.state.configDoc;

    if (useJsonConfig)
        ffPrintJsonConfig(true /* prepare */, instance.state.resultDoc);
    else
        ffPrepareCommandOption(data);

    ffStart();

    #if defined(_WIN32)
        if (!instance.config.display.noBuffer) fflush(stdout);
    #endif

    if (useJsonConfig)
        ffPrintJsonConfig(false, instance.state.resultDoc);
    else
        ffPrintCommandOption(data, instance.state.resultDoc);

    if (instance.state.resultDoc)
        yyjson_mut_write_fp(stdout, instance.state.resultDoc, YYJSON_WRITE_INF_AND_NAN_AS_NULL | YYJSON_WRITE_PRETTY_TWO_SPACES, NULL, NULL);
    else
        ffFinish();
}

static void migrateConfig(FFdata* data, const FFstrbuf* filename)
{
    yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val* root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);
    yyjson_mut_obj_add_str(doc, root, "$schema", "https://github.com/fastfetch-cli/fastfetch/raw/dev/doc/json_schema.json");

    ffOptionsGenerateLogoJsonConfig(&instance.config.logo, doc);
    ffOptionsGenerateDisplayJsonConfig(&instance.config.display, doc);
    ffOptionsGenerateGeneralJsonConfig(&instance.config.general, doc);
    ffOptionsGenerateLibraryJsonConfig(&instance.config.library, doc);
    ffMigrateCommandOptionToJsonc(data, doc);

    if (ffStrbufEqualS(filename, "-"))
        yyjson_mut_write_fp(stdout, doc, YYJSON_WRITE_INF_AND_NAN_AS_NULL | YYJSON_WRITE_PRETTY_TWO_SPACES, NULL, NULL);
    else
    {
        if (yyjson_mut_write_file(filename->chars, doc, YYJSON_WRITE_INF_AND_NAN_AS_NULL | YYJSON_WRITE_PRETTY_TWO_SPACES, NULL, NULL))
            printf("The migrated config file has been written in `%s`\nYou may remove the old config file if everything works fine\n", filename->chars);
        else
        {
            printf("Error: failed to write file in `%s`\n", filename->chars);
            exit(1);
        }
    }

    yyjson_mut_doc_free(doc);
}

int main(int argc, const char** argv)
{
    ffInitInstance();

    //Data stores things only needed for the configuration of fastfetch
    FFdata data = {
        .structure = ffStrbufCreate(),
        .customValues = ffListCreate(sizeof(FFCustomValue)),
        .loadUserConfig = true,
    };

    if(!getenv("NO_CONFIG"))
        parseConfigFiles(&data);
    parseArguments(&data, argc, argv);

    if (__builtin_expect(instance.state.migrateConfigPath.length == 0, true))
        run(&data);
    else
        migrateConfig(&data, &instance.state.migrateConfigPath);

    ffStrbufDestroy(&data.structure);
    FF_LIST_FOR_EACH(FFCustomValue, customValue, data.customValues)
    {
        ffStrbufDestroy(&customValue->key);
        ffStrbufDestroy(&customValue->value);
    }
    ffListDestroy(&data.customValues);

    ffDestroyInstance();
}
