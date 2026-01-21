#include "common/commandoption.h"
#include "common/color.h"
#include "common/printing.h"
#include "common/time.h"
#include "common/jsonconfig.h"
#include "common/stringUtils.h"
#include "fastfetch_datatext.h"
#include "modules/modules.h"

#include <ctype.h>
#include <inttypes.h>

bool ffParseModuleOptions(const char* key, const char* value)
{
    if (!ffStrStartsWith(key, "--") || !ffCharIsEnglishAlphabet(key[2])) return false;
    if (value && !*value) value = NULL;
    for (FFModuleBaseInfo** modules = ffModuleInfos[toupper(key[2]) - 'A']; *modules; ++modules)
    {
        FFModuleBaseInfo* baseInfo = *modules;
        const char* subKey = ffOptionTestPrefix(key, baseInfo->name);
        if (subKey != NULL)
        {
            if (subKey[0] == '\0' || subKey[0] == '-') // Key is exactly the module name or has a leading '-'
            {
                fprintf(stderr, "Error: unknown module key %s\n", key);
                exit(477);
            }

            FF_STRBUF_AUTO_DESTROY moduleName = ffStrbufCreateS(baseInfo->name);
            ffStrbufLowerCase(&moduleName);

            FF_STRBUF_AUTO_DESTROY jsonKey = ffStrbufCreate();
            bool flag = false;
            for (const char* p = subKey; *p; ++p)
            {
                if (*p == '-')
                {
                    if (flag)
                    {
                        fprintf(stderr, "Error: invalid double `-` in module key %s\n", key);
                        exit(477);
                    }
                    flag = true;
                }
                else
                {
                    if (!isalpha((unsigned char)*p) && !isdigit((unsigned char)*p))
                    {
                        fprintf(stderr, "Error: invalid character `%c` in module key %s\n", *p, key);
                        exit(477);
                    }

                    if (flag)
                    {
                        flag = false;
                        ffStrbufAppendC(&jsonKey, (char) toupper((unsigned char) *p));
                    }
                    else
                        ffStrbufAppendC(&jsonKey, *p);
                }
            }
            fprintf(stderr, "Error: Unsupported module option: %s\n", key);
            fputs("       Support of module options has been removed. Please add the flag to the JSON config instead.\n", stderr);
            fprintf(stderr, "       Example (demonstration only): `{ \"modules\": [ { \"type\": \"%s\", \"%s\": %s%s%s } ] }`\n", moduleName.chars, jsonKey.chars, value ? "\"" : "", value ? value : "true", value ? "\"" : "");
            fputs("       See <https://github.com/fastfetch-cli/fastfetch/wiki/Configuration> for more information.\n", stderr);
            exit(477);
        }
    }
    return false;
}

void ffPrepareCommandOption(FFdata* data)
{
    char* moduleType = NULL;
    size_t moduleLen = 0;
    while (ffStrbufGetdelim(&moduleType, &moduleLen, ':', &data->structure))
    {
        #define FF_IF_MODULE_MATCH(moduleNameConstant) if (moduleLen == strlen(moduleNameConstant) \
            && ffStrEqualsIgnCase(moduleType, moduleNameConstant) \
            && !ffStrbufSeparatedContainIgnCaseS(&data->structureDisabled, moduleNameConstant, ':'))

        switch (moduleType[0])
        {
            case 'C': case 'c':
                FF_IF_MODULE_MATCH(FF_CPUUSAGE_MODULE_NAME)
                    ffPrepareCPUUsage();
                break;

            case 'D': case 'd':
                FF_IF_MODULE_MATCH(FF_DISKIO_MODULE_NAME)
                {
                    __attribute__((__cleanup__(ffDestroyDiskIOOptions))) FFDiskIOOptions options;
                    ffInitDiskIOOptions(&options);
                    ffPrepareDiskIO(&options);
                }
                break;

            case 'N': case 'n':
                FF_IF_MODULE_MATCH(FF_NETIO_MODULE_NAME)
                {
                    __attribute__((__cleanup__(ffDestroyNetIOOptions))) FFNetIOOptions options;
                    ffInitNetIOOptions(&options);
                    ffPrepareNetIO(&options);
                }
                break;

            case 'P': case 'p':
                FF_IF_MODULE_MATCH(FF_PUBLICIP_MODULE_NAME)
                {
                    __attribute__((__cleanup__(ffDestroyPublicIpOptions))) FFPublicIPOptions options;
                    ffInitPublicIpOptions(&options);
                    ffPreparePublicIp(&options);
                }
                break;

            case 'W': case 'w':
                FF_IF_MODULE_MATCH(FF_WEATHER_MODULE_NAME)
                {
                    __attribute__((__cleanup__(ffDestroyWeatherOptions))) FFWeatherOptions options;
                    ffInitWeatherOptions(&options);
                    ffPrepareWeather(&options);
                }
                break;
        }

        #undef FF_IF_MODULE_MATCH
    }
}

static void genJsonConfig(FFdata* data, FFModuleBaseInfo* baseInfo, void* options)
{
    yyjson_mut_doc* doc = data->resultDoc;

    yyjson_mut_val* modules = yyjson_mut_obj_get(doc->root, "modules");
    if (!modules)
        modules = yyjson_mut_obj_add_arr(doc, doc->root, "modules");

    FF_STRBUF_AUTO_DESTROY type = ffStrbufCreateS(baseInfo->name);
    ffStrbufLowerCase(&type);

    if (data->docType == FF_RESULT_DOC_TYPE_CONFIG_FULL)
    {
        yyjson_mut_val* module = yyjson_mut_obj(doc);
        yyjson_mut_obj_add_strbuf(doc, module, "type", &type);

        if (baseInfo->generateJsonConfig)
            baseInfo->generateJsonConfig(options, doc, module);

        if (yyjson_mut_obj_size(module) > 1)
            yyjson_mut_arr_add_val(modules, module);
        else
            yyjson_mut_arr_add_strbuf(doc, modules, &type);
    }
    else
    {
        yyjson_mut_arr_add_strbuf(doc, modules, &type);
    }
}

static void genJsonResult(FFdata* data, FFModuleBaseInfo* baseInfo, void* options)
{
    yyjson_mut_doc* doc = data->resultDoc;
    yyjson_mut_val* module = yyjson_mut_arr_add_obj(doc, doc->root);
    yyjson_mut_obj_add_str(doc, module, "type", baseInfo->name);
    if (baseInfo->generateJsonResult)
        baseInfo->generateJsonResult(options, doc, module);
    else
        yyjson_mut_obj_add_str(doc, module, "error", "Unsupported for JSON format");
}

static void parseStructureCommand(
    FFdata* data,
    const char* line,
    void (*fn)(FFdata*, FFModuleBaseInfo* baseInfo, void* options)
)
{
    if(ffCharIsEnglishAlphabet(line[0]))
    {
        for (FFModuleBaseInfo** modules = ffModuleInfos[toupper(line[0]) - 'A']; *modules; ++modules)
        {
            FFModuleBaseInfo* baseInfo = *modules;
            if (ffStrEqualsIgnCase(line, baseInfo->name))
            {
                uint8_t optionBuf[FF_OPTION_MAX_SIZE];
                baseInfo->initOptions(optionBuf);
                if (__builtin_expect(data->resultDoc != NULL, false))
                    fn(data, baseInfo, optionBuf);
                else
                    baseInfo->printModule(optionBuf);
                baseInfo->destroyOptions(optionBuf);
                return;
            }
        }
    }

    ffPrintError(line, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "<no implementation provided>");
}

void ffPrintCommandOption(FFdata* data)
{
    //Parse the structure and call the modules
    int32_t thres = instance.config.display.stat;

    char* moduleType = NULL;
    size_t moduleLen = 0;
    while (ffStrbufGetdelim(&moduleType, &moduleLen, ':', &data->structure))
    {
        if (ffStrbufSeparatedContainIgnCaseS(&data->structureDisabled, moduleType, ':'))
            continue;

        double ms = 0;
        if(thres >= 0)
            ms = ffTimeGetTick();

        parseStructureCommand(data, moduleType, genJsonResult);

        if(thres >= 0)
        {
            ms = ffTimeGetTick() - ms;

            if (data->resultDoc)
            {
                yyjson_mut_val* moduleJson = yyjson_mut_arr_get_last(data->resultDoc->root);
                yyjson_mut_obj_add_real(data->resultDoc, moduleJson, "stat", ms);
            }
            else
            {
                char str[64];
                int len = snprintf(str, sizeof str, "%.3fms", ms);
                if (thres > 0)
                    snprintf(str, sizeof str, "\e[%sm%.3fms\e[m", (ms <= thres ? FF_COLOR_FG_GREEN : ms <= 2 * thres ? FF_COLOR_FG_YELLOW : FF_COLOR_FG_RED), ms);
                printf("\e7\e[1A\e[9999999C\e[%dD%s\e8", len, str); // Save; Up 1; Right 9999999; Left <len>; Print <str>; Load
            }
        }

        #if defined(_WIN32)
            if (!data->resultDoc && !instance.config.display.noBuffer) fflush(stdout);
        #endif
    }
}

void ffMigrateCommandOptionToJsonc(FFdata* data)
{
    //If we don't have a custom structure, use the default one
    if(data->structure.length == 0)
        ffStrbufAppendS(&data->structure, FASTFETCH_DATATEXT_STRUCTURE); // Cannot use `ffStrbufSetStatic` here because we will modify the string

    char* moduleType = NULL;
    size_t moduleLen = 0;
    while (ffStrbufGetdelim(&moduleType, &moduleLen, ':', &data->structure))
    {
        if (ffStrbufSeparatedContainIgnCaseS(&data->structureDisabled, moduleType, ':'))
            continue;

        parseStructureCommand(data, moduleType, genJsonConfig);
    }
}
