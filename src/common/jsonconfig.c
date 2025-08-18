#include "fastfetch.h"
#include "common/color.h"
#include "common/jsonconfig.h"
#include "common/printing.h"
#include "common/io/io.h"
#include "common/time.h"
#include "detection/version/version.h"
#include "modules/modules.h"
#include "util/stringUtils.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>

bool ffJsonConfigParseModuleArgs(yyjson_val* key, yyjson_val* val, FFModuleArgs* moduleArgs)
{
    if (unsafe_yyjson_equals_str(key, "type") || unsafe_yyjson_equals_str(key, "condition"))
        return true;

    if (unsafe_yyjson_equals_str(key, "key"))
    {
        ffStrbufSetJsonVal(&moduleArgs->key, val);
        return true;
    }
    else if (unsafe_yyjson_equals_str(key, "format"))
    {
        ffStrbufSetJsonVal(&moduleArgs->outputFormat, val);
        return true;
    }
    else if (unsafe_yyjson_equals_str(key, "outputColor"))
    {
        ffOptionParseColor(yyjson_get_str(val), &moduleArgs->outputColor);
        return true;
    }
    else if (unsafe_yyjson_equals_str(key, "keyColor"))
    {
        ffOptionParseColor(yyjson_get_str(val), &moduleArgs->keyColor);
        return true;
    }
    else if (unsafe_yyjson_equals_str(key, "keyWidth"))
    {
        moduleArgs->keyWidth = (uint32_t) yyjson_get_uint(val);
        return true;
    }
    else if (unsafe_yyjson_equals_str(key, "keyIcon"))
    {
        ffStrbufSetJsonVal(&moduleArgs->keyIcon, val);
        return true;
    }
    return false;
}

void ffJsonConfigGenerateModuleArgsConfig(yyjson_mut_doc* doc, yyjson_mut_val* module, FFModuleArgs* moduleArgs)
{
    if (moduleArgs->key.length > 0)
        yyjson_mut_obj_add_strbuf(doc, module, "key", &moduleArgs->key);
    if (moduleArgs->outputFormat.length > 0)
        yyjson_mut_obj_add_strbuf(doc, module, "format", &moduleArgs->outputFormat);
    if (moduleArgs->outputColor.length > 0)
        yyjson_mut_obj_add_strbuf(doc, module, "outputColor", &moduleArgs->outputColor);
    if (moduleArgs->keyColor.length > 0)
        yyjson_mut_obj_add_strbuf(doc, module, "keyColor", &moduleArgs->keyColor);
    if (moduleArgs->keyWidth > 0)
        yyjson_mut_obj_add_uint(doc, module, "keyWidth", moduleArgs->keyWidth);
    if (moduleArgs->keyIcon.length > 0)
        yyjson_mut_obj_add_strbuf(doc, module, "keyIcon", &moduleArgs->keyIcon);
}

const char* ffJsonConfigParseEnum(yyjson_val* val, int* result, FFKeyValuePair pairs[])
{
    if (yyjson_is_int(val))
    {
        int intVal = yyjson_get_int(val);

        for (const FFKeyValuePair* pPair = pairs; pPair->key; ++pPair)
        {
            if (intVal == pPair->value)
            {
                *result = pPair->value;
                return NULL;
            }
        }

        return "Invalid enum integer";
    }
    else if (yyjson_is_str(val))
    {
        const char* strVal = yyjson_get_str(val);
        for (const FFKeyValuePair* pPair = pairs; pPair->key; ++pPair)
        {
            if (ffStrEqualsIgnCase(strVal, pPair->key))
            {
                *result = pPair->value;
                return NULL;
            }
        }

        return "Invalid enum string";
    }
    else
        return "Invalid enum value type; must be a string or integer";
}

static bool parseModuleJsonObject(const char* type, yyjson_val* jsonVal, yyjson_mut_doc* jsonDoc)
{
    if(!ffCharIsEnglishAlphabet(type[0])) return false;

    for (FFModuleBaseInfo** modules = ffModuleInfos[toupper(type[0]) - 'A']; *modules; ++modules)
    {
        FFModuleBaseInfo* baseInfo = *modules;
        if (ffStrEqualsIgnCase(type, baseInfo->name))
        {
            uint8_t optionBuf[FF_OPTION_MAX_SIZE];
            baseInfo->initOptions(optionBuf);
            if (jsonVal) baseInfo->parseJsonObject(optionBuf, jsonVal);
            bool succeeded;
            if (jsonDoc)
            {
                yyjson_mut_val* module = yyjson_mut_arr_add_obj(jsonDoc, jsonDoc->root);
                yyjson_mut_obj_add_str(jsonDoc, module, "type", baseInfo->name);
                if (baseInfo->generateJsonResult)
                    succeeded = baseInfo->generateJsonResult(optionBuf, jsonDoc, module);
                else
                {
                    yyjson_mut_obj_add_str(jsonDoc, module, "error", "Unsupported for JSON format");
                    succeeded = false;
                }
            }
            else
                succeeded = baseInfo->printModule(optionBuf);
            baseInfo->destroyOptions(optionBuf);
            return succeeded;
        }
    }

    if (jsonDoc)
    {
        yyjson_mut_val* module = yyjson_mut_arr_add_obj(jsonDoc, jsonDoc->root);
        yyjson_mut_obj_add_strcpy(jsonDoc, module, "type", type);
        yyjson_mut_obj_add_str(jsonDoc, module, "error", "Unknown module type");
    }
    else
    {
        FFModuleArgs moduleArgs;
        ffOptionInitModuleArg(&moduleArgs, "");
        ffPrintError(type, 0, &moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown module type");
        ffOptionDestroyModuleArg(&moduleArgs);
    }
    return false;
}

static void prepareModuleJsonObject(const char* type, yyjson_val* module)
{
    switch (type[0])
    {
        case 'b': case 'B': {
            if (ffStrEqualsIgnCase(type, FF_CPUUSAGE_MODULE_NAME))
                ffPrepareCPUUsage();
            break;
        }
        case 'd': case 'D': {
            if (ffStrEqualsIgnCase(type, FF_DISKIO_MODULE_NAME))
            {
                __attribute__((__cleanup__(ffDestroyDiskIOOptions))) FFDiskIOOptions options;
                ffInitDiskIOOptions(&options);
                if (module) ffDiskIOModuleInfo.parseJsonObject(&options, module);
                ffPrepareDiskIO(&options);
            }
            break;
        }
        case 'n': case 'N': {
            if (ffStrEqualsIgnCase(type, FF_NETIO_MODULE_NAME))
            {
                __attribute__((__cleanup__(ffDestroyNetIOOptions))) FFNetIOOptions options;
                ffInitNetIOOptions(&options);
                if (module) ffNetIOModuleInfo.parseJsonObject(&options, module);
                ffPrepareNetIO(&options);
            }
            break;
        }
        case 'p': case 'P': {
            if (ffStrEqualsIgnCase(type, FF_PUBLICIP_MODULE_NAME))
            {
                __attribute__((__cleanup__(ffDestroyPublicIpOptions))) FFPublicIPOptions options;
                ffInitPublicIpOptions(&options);
                if (module) ffPublicIPModuleInfo.parseJsonObject(&options, module);
                ffPreparePublicIp(&options);
            }
            break;
        }
        case 'w': case 'W': {
            if (ffStrEqualsIgnCase(type, FF_WEATHER_MODULE_NAME))
            {
                __attribute__((__cleanup__(ffDestroyWeatherOptions))) FFWeatherOptions options;
                ffInitWeatherOptions(&options);
                if (module) ffWeatherModuleInfo.parseJsonObject(&options, module);
                ffPrepareWeather(&options);
            }
            break;
        }
    }
}

static bool matchesJsonArray(const char* str, yyjson_val* val)
{
    assert(val);

    if (unsafe_yyjson_is_str(val))
        return ffStrEqualsIgnCase(str, unsafe_yyjson_get_str(val));

    if (!unsafe_yyjson_is_arr(val)) return false;

    size_t idx, max;
    yyjson_val* item;
    yyjson_arr_foreach(val, idx, max, item)
    {
        if (yyjson_is_str(item) && ffStrEqualsIgnCase(str, unsafe_yyjson_get_str(item)))
            return true;
    }
    return false;
}

static const char* printJsonConfig(bool prepare, yyjson_mut_doc* jsonDoc)
{
    yyjson_val* const root = yyjson_doc_get_root(instance.state.configDoc);
    assert(root);

    if (!yyjson_is_obj(root))
        return "Invalid JSON config format. Root value must be an object";

    yyjson_val* modules = yyjson_obj_get(root, "modules");
    if (!modules) return NULL;
    if (!yyjson_is_arr(modules)) return "Property 'modules' must be an array of strings or objects";

    bool succeeded = true;
    int32_t thres = instance.config.display.stat;
    yyjson_val* item;
    size_t idx, max;
    yyjson_arr_foreach(modules, idx, max, item)
    {
        double ms = 0;
        if(!prepare && thres >= 0)
            ms = ffTimeGetTick();

        yyjson_val* module = item;
        const char* type = yyjson_get_str(module);
        if (type)
            module = NULL;
        else if (yyjson_is_obj(module))
        {
            yyjson_val* conditions = yyjson_obj_get(module, "condition");
            if (conditions)
            {
                if (!yyjson_is_obj(conditions))
                    return "Property 'conditions' must be an object";

                yyjson_val* system = yyjson_obj_get(conditions, "system");
                if (system && !matchesJsonArray(ffVersionResult.sysName, system))
                    continue;

                system = yyjson_obj_get(conditions, "!system");
                if (system && matchesJsonArray(ffVersionResult.sysName, system))
                    continue;

                yyjson_val* arch = yyjson_obj_get(conditions, "arch");
                if (arch && !matchesJsonArray(ffVersionResult.architecture, arch))
                    continue;

                arch = yyjson_obj_get(conditions, "!arch");
                if (arch && matchesJsonArray(ffVersionResult.architecture, arch))
                    continue;

                yyjson_val* previousSucceeded = yyjson_obj_get(conditions, "succeeded");
                if (previousSucceeded && !unsafe_yyjson_is_null(previousSucceeded))
                {
                    if (!unsafe_yyjson_is_bool(previousSucceeded))
                        return "Property 'succeeded' in 'condition' must be a boolean";
                    if (succeeded != unsafe_yyjson_get_bool(previousSucceeded))
                        continue;
                }
            }

            type = yyjson_get_str(yyjson_obj_get(module, "type"));
            if (!type) return "module object must contain a \"type\" key ( case sensitive )";
            if (yyjson_obj_size(module) == 1) // contains only Property type
                module = NULL;
        }
        else
            return "modules must be an array of strings or objects";

        if(prepare)
            prepareModuleJsonObject(type, module);
        else
            succeeded = parseModuleJsonObject(type, module, jsonDoc);

        if(!prepare && thres >= 0)
        {
            ms = ffTimeGetTick() - ms;
            if (jsonDoc)
            {
                yyjson_mut_val* moduleJson = yyjson_mut_arr_get_last(jsonDoc->root);
                yyjson_mut_obj_add_real(jsonDoc, moduleJson, "stat", ms);
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
        if (!instance.config.display.noBuffer && !jsonDoc) fflush(stdout);
        #endif
    }

    return NULL;
}

void ffPrintJsonConfig(bool prepare, yyjson_mut_doc* jsonDoc)
{
    const char* error = printJsonConfig(prepare, jsonDoc);
    if (error)
    {
        if (jsonDoc)
        {
            yyjson_mut_val* obj = yyjson_mut_obj(jsonDoc);
            yyjson_mut_obj_add_str(jsonDoc, obj, "error", error);
            yyjson_mut_doc_set_root(jsonDoc, obj);
        }
        else
            ffPrintError("JsonConfig", 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "%s", error);
    }
}
