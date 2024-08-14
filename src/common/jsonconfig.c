#include "fastfetch.h"
#include "common/color.h"
#include "common/jsonconfig.h"
#include "common/printing.h"
#include "common/io/io.h"
#include "common/time.h"
#include "modules/modules.h"
#include "util/stringUtils.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>

bool ffJsonConfigParseModuleArgs(const char* key, yyjson_val* val, FFModuleArgs* moduleArgs)
{
    if(ffStrEqualsIgnCase(key, "key"))
    {
        ffStrbufSetNS(&moduleArgs->key, (uint32_t) yyjson_get_len(val), yyjson_get_str(val));
        return true;
    }
    else if(ffStrEqualsIgnCase(key, "format"))
    {
        ffStrbufSetNS(&moduleArgs->outputFormat, (uint32_t) yyjson_get_len(val), yyjson_get_str(val));
        return true;
    }
    else if(ffStrEqualsIgnCase(key, "outputColor"))
    {
        ffOptionParseColor(yyjson_get_str(val), &moduleArgs->outputColor);
        return true;
    }
    else if(ffStrEqualsIgnCase(key, "keyColor"))
    {
        ffOptionParseColor(yyjson_get_str(val), &moduleArgs->keyColor);
        return true;
    }
    else if(ffStrEqualsIgnCase(key, "keyWidth"))
    {
        moduleArgs->keyWidth = (uint32_t) yyjson_get_uint(val);
        return true;
    }
    else if(ffStrEqualsIgnCase(key, "keyIcon"))
    {
        ffStrbufSetS(&moduleArgs->keyIcon, yyjson_get_str(val));
        return true;
    }
    return false;
}

void ffJsonConfigGenerateModuleArgsConfig(yyjson_mut_doc* doc, yyjson_mut_val* module, FFModuleArgs* defaultModuleArgs, FFModuleArgs* moduleArgs)
{
    if (!ffStrbufEqual(&defaultModuleArgs->key, &moduleArgs->key))
        yyjson_mut_obj_add_strbuf(doc, module, "key", &moduleArgs->key);
    if (!ffStrbufEqual(&defaultModuleArgs->outputFormat, &moduleArgs->outputFormat))
        yyjson_mut_obj_add_strbuf(doc, module, "format", &moduleArgs->outputFormat);
    if (!ffStrbufEqual(&defaultModuleArgs->keyColor, &moduleArgs->keyColor))
        yyjson_mut_obj_add_strbuf(doc, module, "keyColor", &moduleArgs->keyColor);
    if (moduleArgs->keyWidth != defaultModuleArgs->keyWidth)
        yyjson_mut_obj_add_uint(doc, module, "keyWidth", moduleArgs->keyWidth);
    if (!ffStrbufEqual(&defaultModuleArgs->keyIcon, &moduleArgs->keyIcon))
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

static inline void genJsonResult(FFModuleBaseInfo* baseInfo, yyjson_mut_doc* doc)
{
    yyjson_mut_val* module = yyjson_mut_arr_add_obj(doc, doc->root);
    yyjson_mut_obj_add_str(doc, module, "type", baseInfo->name);
    if (baseInfo->generateJsonResult)
        baseInfo->generateJsonResult(baseInfo, doc, module);
    else
        yyjson_mut_obj_add_str(doc, module, "error", "Unsupported for JSON format");
}

static bool parseModuleJsonObject(const char* type, yyjson_val* jsonVal, yyjson_mut_doc* jsonDoc)
{
    if(!ffCharIsEnglishAlphabet(type[0])) return false;

    for (FFModuleBaseInfo** modules = ffModuleInfos[toupper(type[0]) - 'A']; *modules; ++modules)
    {
        FFModuleBaseInfo* baseInfo = *modules;
        if (ffStrEqualsIgnCase(type, baseInfo->name))
        {
            if (jsonVal) baseInfo->parseJsonObject(baseInfo, jsonVal);
            if (__builtin_expect(jsonDoc != NULL, false))
                genJsonResult(baseInfo, jsonDoc);
            else
                baseInfo->printModule(baseInfo);
            return true;
        }
    }
    return false;
}

static void prepareModuleJsonObject(const char* type, yyjson_val* module)
{
    FFconfig* cfg = &instance.config;
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
                if (module) cfg->modules.diskIo.moduleInfo.parseJsonObject(&cfg->modules.diskIo, module);
                ffPrepareDiskIO(&cfg->modules.diskIo);
            }
            break;
        }
        case 'n': case 'N': {
            if (ffStrEqualsIgnCase(type, FF_NETIO_MODULE_NAME))
            {
                if (module) cfg->modules.netIo.moduleInfo.parseJsonObject(&cfg->modules.netIo, module);
                ffPrepareNetIO(&cfg->modules.netIo);
            }
            break;
        }
        case 'p': case 'P': {
            if (ffStrEqualsIgnCase(type, FF_PUBLICIP_MODULE_NAME))
            {
                if (module) cfg->modules.publicIP.moduleInfo.parseJsonObject(&cfg->modules.publicIP, module);
                ffPreparePublicIp(&cfg->modules.publicIP);
            }
            break;
        }
        case 'w': case 'W': {
            if (ffStrEqualsIgnCase(type, FF_WEATHER_MODULE_NAME))
            {
                if (module) cfg->modules.weather.moduleInfo.parseJsonObject(&cfg->modules.weather, module);
                ffPrepareWeather(&cfg->modules.weather);
            }
            break;
        }
    }
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
            type = yyjson_get_str(yyjson_obj_get(module, "type"));
            if (!type) return "module object must contain a \"type\" key ( case sensitive )";
            if (yyjson_obj_size(module) == 1) // contains only Property type
                module = NULL;
        }
        else
            return "modules must be an array of strings or objects";

        if(prepare)
            prepareModuleJsonObject(type, module);
        else if(!parseModuleJsonObject(type, module, jsonDoc))
            return "Unknown module type";

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
                printf("\e[s\e[1A\e[9999999C\e[%dD%s\e[u", len, str); // Save; Up 1; Right 9999999; Left <len>; Print <str>; Load
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
