#include "fastfetch.h"
#include "common/commandoption.h"
#include "common/io/io.h"
#include "common/jsonconfig.h"
#include "detection/version/version.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"
#include "fastfetch_datatext.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef WIN32
    #include "util/windows/getline.h"
#endif

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

static void printFullHelp()
{
    fputs("Fastfetch is a neofetch-like tool for fetching system information and displaying them in a pretty way\n\n", stdout);
    if (!instance.config.display.pipe)
        fputs("\e[1;4mUsage:\e[m \e[1mfastfetch\e[m \e[3m<?options>\e[m\n\n", stdout);
    else
        fputs("Usage: fastfetch <?options>\n\n", stdout);

    yyjson_doc* doc = yyjson_read(FASTFETCH_DATATEXT_JSON_HELP, strlen(FASTFETCH_DATATEXT_JSON_HELP), YYJSON_READ_NOFLAG);
    assert(doc);
    yyjson_val *groupKey, *flagArr;
    size_t groupIdx, groupMax;
    yyjson_obj_foreach(yyjson_doc_get_root(doc), groupIdx, groupMax, groupKey, flagArr)
    {
        if (!instance.config.display.pipe)
            fputs("\e[1;4m", stdout);
        printf("%s options:", yyjson_get_str(groupKey));
        if (!instance.config.display.pipe)
            fputs("\e[m", stdout);
        putchar('\n');

        yyjson_val* flagObj;
        size_t flagIdx, flagMax;
        yyjson_arr_foreach(flagArr, flagIdx, flagMax, flagObj)
        {
            yyjson_val* shortKey = yyjson_obj_get(flagObj, "short");
            if (shortKey)
            {
                fputs("  ", stdout);
                if (!instance.config.display.pipe)
                    fputs("\e[1m", stdout);
                printf("-%s", yyjson_get_str(shortKey));
                if (!instance.config.display.pipe)
                    fputs("\e[m", stdout);
                fputs(", ", stdout);
            }
            else
            {
                fputs("      ", stdout);
            }
            yyjson_val* longKey = yyjson_obj_get(flagObj, "long");
            assert(longKey);
            if (!instance.config.display.pipe)
                fputs("\e[1m", stdout);
            printf("--%s", yyjson_get_str(longKey));
            if (!instance.config.display.pipe)
                fputs("\e[m", stdout);

            yyjson_val* argObj = yyjson_obj_get(flagObj, "arg");
            if (argObj)
            {
                yyjson_val* typeKey = yyjson_obj_get(argObj, "type");
                assert(typeKey);
                yyjson_val* optionalKey = yyjson_obj_get(argObj, "optional");
                bool optional = optionalKey && yyjson_get_bool(optionalKey);
                putchar(' ');
                if (!instance.config.display.pipe)
                    fputs("\e[3m", stdout);
                printf("<%s%s>", optional ? "?" : "", yyjson_get_str(typeKey));
                if (!instance.config.display.pipe)
                    fputs("\e[m", stdout);
            }

            yyjson_val* descKey = yyjson_obj_get(flagObj, "desc");
            assert(descKey);
            if (yyjson_is_arr(descKey))
            {
                if (instance.config.display.pipe)
                    putchar(':');

                yyjson_val* descStr;
                size_t descIdx, descMax;
                yyjson_arr_foreach(descKey, descIdx, descMax, descStr)
                {
                    if (!instance.config.display.pipe)
                        printf("\e[46G%s\n", yyjson_get_str(descStr));
                    else
                        printf(" %s", yyjson_get_str(descStr));
                }
                if (instance.config.display.pipe)
                    putchar('\n');
            }
            else
            {
                if (!instance.config.display.pipe)
                    fputs("\e[46G", stdout);
                else
                    fputs(": ", stdout);
                puts(yyjson_get_str(descKey));
            }
        }

        putchar('\n');
    }
    yyjson_doc_free(doc);

    puts("\n" FASTFETCH_DATATEXT_HELP_FOOTER);
}

static bool printSpecificCommandHelp(const char* command)
{
    yyjson_doc* doc = yyjson_read(FASTFETCH_DATATEXT_JSON_HELP, strlen(FASTFETCH_DATATEXT_JSON_HELP), YYJSON_READ_NOFLAG);
    assert(doc);
    yyjson_val *groupKey, *flagArr;
    size_t groupIdx, groupMax;
    yyjson_obj_foreach(yyjson_doc_get_root(doc), groupIdx, groupMax, groupKey, flagArr)
    {
        yyjson_val* flagObj;
        size_t flagIdx, flagMax;
        yyjson_arr_foreach(flagArr, flagIdx, flagMax, flagObj)
        {
            yyjson_val* pseudo = yyjson_obj_get(flagObj, "pseudo");
            if (pseudo && yyjson_get_bool(pseudo))
                continue;

            yyjson_val* longKey = yyjson_obj_get(flagObj, "long");
            assert(longKey);
            if (ffStrEqualsIgnCase(command, yyjson_get_str(longKey)))
            {
                puts(yyjson_get_str(yyjson_obj_get(flagObj, "desc")));

                printf("%10s: ", "Usage");
                yyjson_val* shortKey = yyjson_obj_get(flagObj, "short");
                if (shortKey)
                {
                    if (!instance.config.display.pipe)
                        fputs("\e[1m", stdout);
                    printf("-%s", yyjson_get_str(shortKey));
                    if (!instance.config.display.pipe)
                        fputs("\e[m", stdout);
                    fputs(", ", stdout);
                }
                if (!instance.config.display.pipe)
                    fputs("\e[1m", stdout);
                printf("--%s", yyjson_get_str(longKey));
                if (!instance.config.display.pipe)
                    fputs("\e[m", stdout);

                yyjson_val* argObj = yyjson_obj_get(flagObj, "arg");
                if (argObj)
                {
                    yyjson_val* typeKey = yyjson_obj_get(argObj, "type");
                    assert(typeKey);
                    yyjson_val* optionalKey = yyjson_obj_get(argObj, "optional");
                    bool optional = optionalKey && yyjson_get_bool(optionalKey);
                    putchar(' ');
                    if (!instance.config.display.pipe)
                        fputs("\e[3m", stdout);
                    printf("<%s%s>", optional ? "?" : "", yyjson_get_str(typeKey));
                    if (!instance.config.display.pipe)
                        fputs("\e[m", stdout);
                    putchar('\n');

                    yyjson_val* defaultKey = yyjson_obj_get(argObj, "default");
                    if (defaultKey)
                    {
                        if (ffStrEqualsIgnCase(yyjson_get_str(typeKey), "structure"))
                            printf("%10s: %s\n", "Default", FASTFETCH_DATATEXT_STRUCTURE);
                        else if (yyjson_is_bool(defaultKey))
                            printf("%10s: %s\n", "Default", yyjson_get_bool(defaultKey) ? "true" : "false");
                        else if (yyjson_is_num(defaultKey))
                            printf("%10s: %d\n", "Default", yyjson_get_int(defaultKey));
                        else if (yyjson_is_str(defaultKey))
                            printf("%10s: %s\n", "Default", yyjson_get_str(defaultKey));
                        else
                            printf("%10s: Unknown\n", "Default");
                    }

                    yyjson_val* enumKey = yyjson_obj_get(argObj, "enum");
                    if (enumKey)
                    {
                        printf("%10s:\n", "Options");
                        yyjson_val *optKey, *optVal;
                        size_t optIdx, optMax;
                        yyjson_obj_foreach(enumKey, optIdx, optMax, optKey, optVal)
                            printf("%12s: %s\n", yyjson_get_str(optKey), yyjson_get_str(optVal));
                    }
                }
                else
                    putchar('\n');

                yyjson_val* remarkKey = yyjson_obj_get(flagObj, "remark");
                if (remarkKey)
                {
                    if (yyjson_is_str(remarkKey))
                        printf("%10s: %s\n", "Remark", yyjson_get_str(remarkKey));
                    else if (yyjson_is_arr(remarkKey) && yyjson_arr_size(remarkKey) > 0)
                    {
                        yyjson_val* remarkStr;
                        size_t remarkIdx, remarkMax;
                        yyjson_arr_foreach(remarkKey, remarkIdx, remarkMax, remarkStr)
                        {
                            if (remarkIdx == 0)
                                printf("%10s: %s\n", "Remark", yyjson_get_str(remarkStr));
                            else
                                printf("            %s\n", yyjson_get_str(remarkStr));
                        }
                    }
                }

                yyjson_doc_free(doc);
                return true;
            }
        }
    }

    yyjson_doc_free(doc);
    return false;
}

static void printCommandHelp(const char* command)
{
    if(command == NULL)
        printFullHelp();
    else if(ffStrEqualsIgnCase(command, "color"))
        puts(FASTFETCH_DATATEXT_HELP_COLOR);
    else if(ffStrEqualsIgnCase(command, "format"))
        puts(FASTFETCH_DATATEXT_HELP_FORMAT);
    else if(ffCharIsEnglishAlphabet(command[0]) && ffStrEndsWithIgnCase(command, "-format")) // <module>-format
        printCommandFormatHelp(command);
    else if(!printSpecificCommandHelp(command))
        fprintf(stderr, "Error: No specific help for command '%s' provided\n", command);
}

static void listAvailablePresets(bool pretty)
{
    FF_LIST_FOR_EACH(FFstrbuf, path, instance.state.platform.dataDirs)
    {
        ffStrbufAppendS(path, "fastfetch/presets/");
        ffListFilesRecursively(path->chars, pretty);
    }

    FF_STRBUF_AUTO_DESTROY absolutePath = ffStrbufCreateCopy(&instance.state.platform.exePath);
    ffStrbufSubstrBeforeLastC(&absolutePath, '/');
    ffStrbufAppendS(&absolutePath, "/presets/");
    ffListFilesRecursively(absolutePath.chars, pretty);
}

static void listAvailableLogos(void)
{
    FF_LIST_FOR_EACH(FFstrbuf, path, instance.state.platform.dataDirs)
    {
        ffStrbufAppendS(path, "fastfetch/logos/");
        ffListFilesRecursively(path->chars, true);
    }
}

static void listConfigPaths(void)
{
    FF_LIST_FOR_EACH(FFstrbuf, folder, instance.state.platform.configDirs)
    {
        bool exists = false;
        uint32_t length = folder->length + (uint32_t) strlen("fastfetch") + 1 /* trailing slash */;
        ffStrbufAppendS(folder, "fastfetch/config.jsonc");
        exists = ffPathExists(folder->chars, FF_PATHTYPE_FILE);
        ffStrbufSubstrBefore(folder, length);
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

static void listModules(bool pretty)
{
    unsigned count = 0;
    for (int i = 0; i <= 'Z' - 'A'; ++i)
    {
        for (FFModuleBaseInfo** modules = ffModuleInfos[i]; *modules; ++modules)
        {
            ++count;
            if (pretty)
                printf("%d)%s%-14s: %s\n", count, count > 9 ? " " : "  ", (*modules)->name, (*modules)->description);
            else
                printf("%s:%s\n", (*modules)->name, (*modules)->description);
        }
    }
}

// Temporary copy before new release of yyjson
static bool ffyyjson_locate_pos(const char *str, size_t len, size_t pos,
                                size_t *line, size_t *col, size_t *chr) {
    size_t line_sum = 0, line_pos = 0, chr_sum = 0;
    const uint8_t *cur = (const uint8_t *)str;
    const uint8_t *end = cur + pos;

    if (!str || pos > len) {
        if (line) *line = 0;
        if (col) *col = 0;
        if (chr) *chr = 0;
        return false;
    }

    while (cur < end) {
        uint8_t c = *cur;
        chr_sum += 1;
        if (__builtin_expect(c < 0x80, true)) {         /* 0xxxxxxx (0x00-0x7F) ASCII */
            if (c == '\n') {
                line_sum += 1;
                line_pos = chr_sum;
            }
            cur += 1;
        }
        else if (c < 0xC0) cur += 1;    /* 10xxxxxx (0x80-0xBF) Invalid */
        else if (c < 0xE0) cur += 2;    /* 110xxxxx (0xC0-0xDF) 2-byte UTF-8 */
        else if (c < 0xF0) cur += 3;    /* 1110xxxx (0xE0-0xEF) 3-byte UTF-8 */
        else if (c < 0xF8) cur += 4;    /* 11110xxx (0xF0-0xF7) 4-byte UTF-8 */
        else               cur += 1;    /* 11111xxx (0xF8-0xFF) Invalid */
    }

    if (line) *line = line_sum + 1;
    if (col) *col = chr_sum - line_pos + 1;
    if (chr) *chr = chr_sum;
    return true;
}


static bool parseJsoncFile(const char* path)
{
    assert(!instance.state.configDoc);

    {
        yyjson_read_err error;
        instance.state.configDoc = yyjson_read_file(path, YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS, NULL, &error);
        if (!instance.state.configDoc)
        {
            if (error.code != YYJSON_READ_ERROR_FILE_OPEN)
            {
                size_t row = 0, col = error.pos;
                FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
                if (ffAppendFileBuffer(path, &content))
                    ffyyjson_locate_pos(content.chars, content.length, error.pos, &row, &col, NULL);
                fprintf(stderr, "Error: failed to parse JSON config file `%s` at (%zu, %zu): %s\n", path, row, col, error.msg);
                exit(477);
            }
            return false;
        }
    }

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

    return true;
}

static void generateConfigFile(bool force, const char* filePath)
{
    if (!filePath)
    {
        ffStrbufSet(&instance.state.genConfigPath, (FFstrbuf*) ffListGet(&instance.state.platform.configDirs, 0));
        ffStrbufAppendS(&instance.state.genConfigPath, "fastfetch/config.jsonc");
    }
    else
    {
        ffStrbufSetS(&instance.state.genConfigPath, filePath);
    }

    if (!force && ffPathExists(instance.state.genConfigPath.chars, FF_PATHTYPE_ANY))
    {
        fprintf(stderr, "Error: file `%s` exists. Use `--gen-config-force` to overwrite\n", instance.state.genConfigPath.chars);
        exit(477);
    }
}

static void optionParseConfigFile(FFdata* data, const char* key, const char* value)
{
    if (data->configLoaded)
    {
        fprintf(stderr, "Error: only one config file can be loaded\n");
        exit(413);
    }

    data->configLoaded = true;

    if(value == NULL)
    {
        fprintf(stderr, "Error: usage: %s <config>\n", key);
        exit(413);
    }
    uint32_t fileNameLen = (uint32_t) strlen(value);
    if(fileNameLen == 0)
    {
        fprintf(stderr, "Error: usage: %s <config>\n", key);
        exit(413);
    }

    if (ffStrEqualsIgnCase(value, "none"))
        return;

    if (ffStrEndsWithIgnCase(value, ".conf"))
    {
        fprintf(stderr, "Error: flag based config files are no longer not supported: %s\n", value);
        exit(414);
    }

    //Try to load as an absolute path

    if (parseJsoncFile(value)) return;

    //Try to load as a relative path

    FF_STRBUF_AUTO_DESTROY absolutePath = ffStrbufCreateA(128);
    FF_LIST_FOR_EACH(FFstrbuf, path, instance.state.platform.dataDirs)
    {
        //We need to copy it, because if a config file loads a config file, the value of path must be unchanged
        ffStrbufSet(&absolutePath, path);
        ffStrbufAppendS(&absolutePath, "fastfetch/presets/");
        ffStrbufAppendS(&absolutePath, value);

        bool success = parseJsoncFile(absolutePath.chars);
        if (!success)
        {
            ffStrbufAppendS(&absolutePath, ".jsonc");
            success = parseJsoncFile(absolutePath.chars);
        }

        if (success) return;
    }

    {
        //Try exe path
        ffStrbufSet(&absolutePath, &instance.state.platform.exePath);
        ffStrbufSubstrBeforeLastC(&absolutePath, '/');
        ffStrbufAppendS(&absolutePath, "/presets/");
        ffStrbufAppendS(&absolutePath, value);

        bool success = parseJsoncFile(absolutePath.chars);
        if (!success)
        {
            ffStrbufAppendS(&absolutePath, ".jsonc");
            success = parseJsoncFile(absolutePath.chars);
        }

        if (success) return;
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

static void parseCommand(FFdata* data, char* key, char* value)
{
    if(ffStrEqualsIgnCase(key, "-h") || ffStrEqualsIgnCase(key, "--help"))
    {
        printCommandHelp(value);
        exit(0);
    }
    if(ffStrEqualsIgnCase(key, "--help-raw"))
    {
        puts(FASTFETCH_DATATEXT_JSON_HELP);
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
        if(ffStrEndsWithIgnCase(subkey, "structure"))
            puts(FASTFETCH_DATATEXT_STRUCTURE);
        else if(ffStrEqualsIgnCase(subkey, "logos"))
            ffLogoBuiltinPrint();
        else
        {
            fprintf(stderr, "Error: unsupported print option: %s\n", key);
            exit(415);
        }
        exit(0);
    }
    else if(ffStrStartsWithIgnCase(key, "--list-"))
    {
        const char* subkey = key + strlen("--list-");
        if(ffStrEqualsIgnCase(subkey, "modules"))
            listModules(!value || !ffStrEqualsIgnCase(value, "autocompletion"));
        else if(ffStrEqualsIgnCase(subkey, "presets"))
            listAvailablePresets(!value || !ffStrEqualsIgnCase(value, "autocompletion"));
        else if(ffStrEqualsIgnCase(subkey, "config-paths"))
            listConfigPaths();
        else if(ffStrEqualsIgnCase(subkey, "data-paths"))
            listDataPaths();
        else if(ffStrEqualsIgnCase(subkey, "features"))
            ffListFeatures();
        else if(ffStrEqualsIgnCase(subkey, "logos"))
        {
            if (value)
            {
                if (ffStrEqualsIgnCase(value, "autocompletion"))
                    ffLogoBuiltinListAutocompletion();
                else if (ffStrEqualsIgnCase(value, "builtin"))
                    ffLogoBuiltinList();
                else if (ffStrEqualsIgnCase(value, "custom"))
                    listAvailableLogos();
                else
                {
                    fprintf(stderr, "Error: unsupported logo type: %s\n", value);
                    exit(415);
                }
            }
            else
            {
                puts("Builtin logos:");
                ffLogoBuiltinList();
                puts("\nCustom logos:");
                listAvailableLogos();
            }
        }
        else
        {
            fprintf(stderr, "Error: unsupported list option: %s\n", key);
            exit(415);
        }

        exit(0);
    }
    else if(ffStrEqualsIgnCase(key, "--gen-config"))
        generateConfigFile(false, value);
    else if(ffStrEqualsIgnCase(key, "--gen-config-force"))
        generateConfigFile(true, value);
    else if(ffStrEqualsIgnCase(key, "-c") || ffStrEqualsIgnCase(key, "--load-config") || ffStrEqualsIgnCase(key, "--config"))
        optionParseConfigFile(data, key, value);
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
    else
        return;

    // Don't parse it again in parseOption.
    // This is necessary because parseOption doesn't understand this option and will result in an unknown option error.
    key[0] = '\0';
    if (value) value[0] = '\0';
}

static void parseOption(FFdata* data, const char* key, const char* value)
{
    if(ffStrEqualsIgnCase(key, "-s") || ffStrEqualsIgnCase(key, "--structure"))
        ffOptionParseString(key, value, &data->structure);

    else if(
        ffOptionsParseGeneralCommandLine(&instance.config.general, key, value) ||
        ffOptionsParseLogoCommandLine(&instance.config.logo, key, value) ||
        ffOptionsParseDisplayCommandLine(&instance.config.display, key, value) ||
        ffOptionsParseLibraryCommandLine(&instance.config.library, key, value) ||
        ffParseModuleOptions(key, value)
    ) {}

    else
    {
        fprintf(stderr, "Error: unknown option: %s\n", key);
        exit(400);
    }
}

static void parseConfigFiles(void)
{
    if (__builtin_expect(instance.state.genConfigPath.length == 0, true))
    {
        FF_LIST_FOR_EACH(FFstrbuf, dir, instance.state.platform.configDirs)
        {
            uint32_t dirLength = dir->length;

            ffStrbufAppendS(dir, "fastfetch/config.jsonc");
            bool success = parseJsoncFile(dir->chars);
            ffStrbufSubstrBefore(dir, dirLength);
            if (success) return;
        }
    }
}

static void parseArguments(FFdata* data, int argc, char** argv, void (*parser)(FFdata* data, char* key, char* value))
{
    for(int i = 1; i < argc; i++)
    {
        const char* key = argv[i];
        if(*key == '\0')
            continue; // has been handled by parseCommand

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
            parser(data, argv[i], NULL);
        }
        else
        {
            parser(data, argv[i], argv[i + 1]);
            ++i;
        }
    }
}

static void run(FFdata* data)
{
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
        yyjson_mut_write_fp(stdout, instance.state.resultDoc, YYJSON_WRITE_INF_AND_NAN_AS_NULL | YYJSON_WRITE_PRETTY_TWO_SPACES | YYJSON_WRITE_NEWLINE_AT_END, NULL, NULL);
    else
        ffFinish();
}

static void writeConfigFile(FFdata* data, const FFstrbuf* filename)
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
        yyjson_mut_write_fp(stdout, doc, YYJSON_WRITE_INF_AND_NAN_AS_NULL | YYJSON_WRITE_PRETTY_TWO_SPACES | YYJSON_WRITE_NEWLINE_AT_END, NULL, NULL);
    else
    {
        size_t len;
        FF_AUTO_FREE const char* str = yyjson_mut_write(doc, YYJSON_WRITE_INF_AND_NAN_AS_NULL | YYJSON_WRITE_PRETTY_TWO_SPACES | YYJSON_WRITE_NEWLINE_AT_END, &len);
        if (!str)
        {
            printf("Error: failed to generate config file\n");
            exit(1);
        }
        if (ffWriteFileData(filename->chars, len, str))
            printf("The generated config file has been written in `%s`\n", filename->chars);
        else
        {
            printf("Error: failed to write file in `%s`\n", filename->chars);
            exit(1);
        }
    }

    yyjson_mut_doc_free(doc);
}

int main(int argc, char** argv)
{
    ffInitInstance();
    atexit(ffDestroyInstance);

    //Data stores things only needed for the configuration of fastfetch
    FFdata data = {
        .structure = ffStrbufCreate(),
        .configLoaded = false,
    };

    parseArguments(&data, argc, argv, parseCommand);
    if(!data.configLoaded && !getenv("NO_CONFIG"))
        parseConfigFiles();
    parseArguments(&data, argc, argv, (void*) parseOption);

    if (__builtin_expect(instance.state.genConfigPath.length == 0, true))
        run(&data);
    else
        writeConfigFile(&data, &instance.state.genConfigPath);

    ffStrbufDestroy(&data.structure);
}
