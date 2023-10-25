#include "fastfetch.h"
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

static inline yyjson_mut_val* genJsonResult(FFModuleBaseInfo* baseInfo)
{
    yyjson_mut_doc* doc = instance.state.resultDoc;
    if (__builtin_expect(!doc, true)) return NULL;

    yyjson_mut_val* module = yyjson_mut_arr_add_obj(doc, doc->root);
    yyjson_mut_obj_add_str(doc, module, "type", baseInfo->name);
    if (baseInfo->generateJsonResult)
        baseInfo->generateJsonResult(baseInfo, doc, module);
    else
        yyjson_mut_obj_add_str(doc, module, "error", "Unsupported for JSON format");
    return module;
}

static bool parseModuleJsonObject(const char* type, yyjson_val* jsonVal)
{
    if(!isalpha(type[0])) return false;

    for (FFModuleBaseInfo** modules = ffModuleInfos[toupper(type[0]) - 'A']; *modules; ++modules)
    {
        FFModuleBaseInfo* baseInfo = *modules;
        if (ffStrEqualsIgnCase(type, baseInfo->name))
        {
            if (jsonVal) baseInfo->parseJsonObject(baseInfo, jsonVal);
            if (!genJsonResult(baseInfo))
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

static const char* printJsonConfig(bool prepare)
{
    yyjson_val* const root = yyjson_doc_get_root(instance.state.configDoc);
    assert(root);

    if (!yyjson_is_obj(root))
        return "Invalid JSON config format. Root value must be an object";

    yyjson_val* modules = yyjson_obj_get(root, "modules");
    if (!modules) return NULL;
    if (!yyjson_is_arr(modules)) return "Property 'modules' must be an array of strings or objects";

    yyjson_mut_doc* resultDoc = instance.state.resultDoc;

    yyjson_val* item;
    size_t idx, max;
    yyjson_arr_foreach(modules, idx, max, item)
    {
        uint64_t ms = 0;
        if(!prepare && instance.config.stat)
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
        else if(!parseModuleJsonObject(type, module))
            return "Unknown module type";

        if(!prepare && instance.config.stat)
        {
            ms = ffTimeGetTick() - ms;
            if (resultDoc)
            {
                yyjson_mut_val* moduleJson = yyjson_mut_arr_get_last(resultDoc->root);
                yyjson_mut_obj_add_uint(resultDoc, moduleJson, "stat", ms);
            }
            else
            {
                char str[32];
                int len = snprintf(str, sizeof str, "%" PRIu64 "ms", ms);
                if(instance.config.pipe)
                    puts(str);
                else
                    printf("\033[s\033[1A\033[9999999C\033[%dD%s\033[u", len, str); // Save; Up 1; Right 9999999; Left <len>; Print <str>; Load
            }
        }

        #if defined(_WIN32)
        if (!instance.config.noBuffer && !resultDoc) fflush(stdout);
        #endif
    }

    return NULL;
}

const char* ffParseDisplayJsonConfig(FFconfig* config)
{
    yyjson_val* const root = yyjson_doc_get_root(instance.state.configDoc);
    assert(root);

    if (!yyjson_is_obj(root))
        return "Invalid JSON config format. Root value must be an object";

    yyjson_val* object = yyjson_obj_get(root, "display");
    if (!object) return NULL;
    if (!yyjson_is_obj(object)) return "Property 'display' must be an object";

    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(object, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);

        if (ffStrEqualsIgnCase(key, "stat"))
        {
            if ((config->stat = yyjson_get_bool(val)))
                config->showErrors = true;
        }
        else if (ffStrEqualsIgnCase(key, "pipe"))
            config->pipe = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "showErrors"))
            config->showErrors = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "disableLinewrap"))
            config->disableLinewrap = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "hideCursor"))
            config->hideCursor = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "separator"))
            ffStrbufSetS(&config->keyValueSeparator, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "color"))
        {
            if (yyjson_is_str(val))
            {
                ffOptionParseColor(yyjson_get_str(val), &config->colorKeys);
                ffStrbufSet(&config->colorTitle, &config->colorKeys);
            }
            else if (yyjson_is_obj(val))
            {
                const char* colorKeys = yyjson_get_str(yyjson_obj_get(val, "keys"));
                if (colorKeys)
                    ffOptionParseColor(colorKeys, &config->colorKeys);
                const char* colorTitle = yyjson_get_str(yyjson_obj_get(val, "title"));
                if (colorTitle)
                    ffOptionParseColor(colorTitle, &config->colorTitle);
            }
            else
                return "display.color must be either a string or an object";
        }
        else if (ffStrEqualsIgnCase(key, "brightColor"))
            config->brightColor = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "binaryPrefix"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "iec", FF_BINARY_PREFIX_TYPE_IEC },
                { "si", FF_BINARY_PREFIX_TYPE_SI },
                { "jedec", FF_BINARY_PREFIX_TYPE_JEDEC },
                {},
            });
            if (error) return error;
            config->binaryPrefixType = (FFBinaryPrefixType) value;
        }
        else if (ffStrEqualsIgnCase(key, "sizeNdigits"))
            config->sizeNdigits = (uint8_t) yyjson_get_uint(val);
        else if (ffStrEqualsIgnCase(key, "sizeMaxPrefix"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
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
            if (error) return error;
            config->sizeMaxPrefix = (uint8_t) value;
        }
        else if (ffStrEqualsIgnCase(key, "temperatureUnit"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "CELSIUS", FF_TEMPERATURE_UNIT_CELSIUS },
                { "C", FF_TEMPERATURE_UNIT_CELSIUS },
                { "FAHRENHEIT", FF_TEMPERATURE_UNIT_FAHRENHEIT },
                { "F", FF_TEMPERATURE_UNIT_FAHRENHEIT },
                { "KELVIN", FF_TEMPERATURE_UNIT_KELVIN },
                { "K", FF_TEMPERATURE_UNIT_KELVIN },
                {},
            });
            if (error) return error;
            config->temperatureUnit = (FFTemperatureUnit) value;
        }
        else if (ffStrEqualsIgnCase(key, "percentType"))
            config->percentType = (uint8_t) yyjson_get_uint(val);
        else if (ffStrEqualsIgnCase(key, "percentNdigits"))
            config->percentNdigits = (uint8_t) yyjson_get_uint(val);
        else if (ffStrEqualsIgnCase(key, "bar"))
        {
            if (yyjson_is_obj(val))
            {
                const char* charElapsed = yyjson_get_str(yyjson_obj_get(val, "charElapsed"));
                if (charElapsed)
                    ffStrbufSetS(&config->barCharElapsed, charElapsed);

                const char* charTotal = yyjson_get_str(yyjson_obj_get(val, "charTotal"));
                if (charTotal)
                    ffStrbufSetS(&config->barCharTotal, charTotal);

                yyjson_val* border = yyjson_obj_get(val, "border");
                if (border)
                    config->barBorder = yyjson_get_bool(border);

                yyjson_val* width = yyjson_obj_get(val, "width");
                if (width)
                    config->barWidth = (uint8_t) yyjson_get_uint(width);
            }
            else
                return "display.bar must be an object";
        }
        else if (ffStrEqualsIgnCase(key, "noBuffer"))
            config->noBuffer = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "keyWidth"))
            config->keyWidth = (uint32_t) yyjson_get_uint(val);
        else
            return "Unknown display property";
    }

    return NULL;
}

const char* ffParseLibraryJsonConfig(FFconfig* config)
{
    yyjson_val* const root = yyjson_doc_get_root(instance.state.configDoc);
    assert(root);

    if (!yyjson_is_obj(root))
        return "Invalid JSON config format. Root value must be an object";

    yyjson_val* object = yyjson_obj_get(root, "library");
    if (!object) return NULL;
    if (!yyjson_is_obj(object)) return "Property 'library' must be an object";

    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(object, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);

        if (ffStrEqualsIgnCase(key, "pci"))
            ffStrbufSetS(&config->libPCI, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "vulkan"))
            ffStrbufSetS(&config->libVulkan, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "freetype"))
            ffStrbufSetS(&config->libfreetype, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "wayland"))
            ffStrbufSetS(&config->libWayland, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "xcbRandr"))
            ffStrbufSetS(&config->libXcbRandr, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "xcb"))
            ffStrbufSetS(&config->libXcb, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "Xrandr"))
            ffStrbufSetS(&config->libXrandr, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "X11"))
            ffStrbufSetS(&config->libX11, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "gio"))
            ffStrbufSetS(&config->libGIO, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "DConf"))
            ffStrbufSetS(&config->libDConf, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "dbus"))
            ffStrbufSetS(&config->libDBus, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "XFConf"))
            ffStrbufSetS(&config->libXFConf, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "sqlite") || ffStrEqualsIgnCase(key, "sqlite3"))
            ffStrbufSetS(&config->libSQLite3, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "rpm"))
            ffStrbufSetS(&config->librpm, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "imagemagick"))
            ffStrbufSetS(&config->libImageMagick, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "z"))
            ffStrbufSetS(&config->libZ, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "chafa"))
            ffStrbufSetS(&config->libChafa, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "egl"))
            ffStrbufSetS(&config->libEGL, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "glx"))
            ffStrbufSetS(&config->libGLX, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "osmesa"))
            ffStrbufSetS(&config->libOSMesa, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "opencl"))
            ffStrbufSetS(&config->libOpenCL, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "pulse"))
            ffStrbufSetS(&config->libPulse, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "nm"))
            ffStrbufSetS(&config->libnm, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "ddcutil"))
            ffStrbufSetS(&config->libDdcutil, yyjson_get_str(val));
        else
            return "Unknown library property";
    }

    return NULL;
}

void ffPrintJsonConfig(bool prepare)
{
    const char* error = printJsonConfig(prepare);
    if (error)
        ffPrintErrorString("JsonConfig", 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "%s", error);
}
