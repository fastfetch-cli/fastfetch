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

static inline bool tryModule(const char* type, yyjson_val* module, void* options)
{
    FFModuleBaseInfo* baseInfo = (FFModuleBaseInfo*) options;
    if (ffStrEqualsIgnCase(type, baseInfo->name))
    {
        if (module) baseInfo->parseJsonObject(options, module);
        baseInfo->printModule(options);
        return true;
    }
    return false;
}

static bool parseModuleJsonObject(const char* type, yyjson_val* module)
{
    FFconfig* cfg = &instance.config;
    switch (toupper(type[0]))
    {
        case 'B': {
            return
                tryModule(type, module, &cfg->battery) ||
                tryModule(type, module, &cfg->bios) ||
                tryModule(type, module, &cfg->bluetooth) ||
                tryModule(type, module, &cfg->board) ||
                tryModule(type, module, &cfg->break_) ||
                tryModule(type, module, &cfg->brightness) ||
                false;
        }

        case 'C': {
            return
                tryModule(type, module, &cfg->chassis) ||
                tryModule(type, module, &cfg->command) ||
                tryModule(type, module, &cfg->colors) ||
                tryModule(type, module, &cfg->cpu) ||
                tryModule(type, module, &cfg->cpuUsage) ||
                tryModule(type, module, &cfg->cursor) ||
                tryModule(type, module, &cfg->custom) ||
                false;
        }

        case 'D': {
            return
                tryModule(type, module, &cfg->dateTime) ||
                tryModule(type, module, &cfg->de) ||
                tryModule(type, module, &cfg->display) ||
                tryModule(type, module, &cfg->disk) ||
                false;
        }

        case 'F': {
            return
                tryModule(type, module, &cfg->font) ||
                false;
        }

        case 'G': {
            return
                tryModule(type, module, &cfg->gamepad) ||
                tryModule(type, module, &cfg->gpu) ||
                false;
        }

        case 'H': {
            return
                tryModule(type, module, &cfg->host) ||
                false;
        }

        case 'I': {
            return
                tryModule(type, module, &cfg->icons) ||
                false;
        }

        case 'K': {
            return
                tryModule(type, module, &cfg->kernel) ||
                false;
        }

        case 'L': {
            return
                tryModule(type, module, &cfg->lm) ||
                tryModule(type, module, &cfg->locale) ||
                tryModule(type, module, &cfg->localIP) ||
                false;
        }

        case 'M': {
            return
                tryModule(type, module, &cfg->media) ||
                tryModule(type, module, &cfg->memory) ||
                tryModule(type, module, &cfg->monitor) ||
                false;
        }

        case 'O': {
            return
                tryModule(type, module, &cfg->openCL) ||
                tryModule(type, module, &cfg->openGL) ||
                tryModule(type, module, &cfg->os) ||
                false;
        }

        case 'P': {
            return
                tryModule(type, module, &cfg->packages) ||
                tryModule(type, module, &cfg->player) ||
                tryModule(type, module, &cfg->powerAdapter) ||
                tryModule(type, module, &cfg->processes) ||
                tryModule(type, module, &cfg->publicIP) ||
                false;
        }

        case 'S': {
            return
                tryModule(type, module, &cfg->separator) ||
                tryModule(type, module, &cfg->shell) ||
                tryModule(type, module, &cfg->sound) ||
                tryModule(type, module, &cfg->swap) ||
                false;
        }

        case 'T': {
            return
                tryModule(type, module, &cfg->terminal) ||
                tryModule(type, module, &cfg->terminalFont) ||
                tryModule(type, module, &cfg->terminalSize) ||
                tryModule(type, module, &cfg->title) ||
                tryModule(type, module, &cfg->theme) ||
                false;
        }

        case 'U': {
            return
                tryModule(type, module, &cfg->uptime) ||
                tryModule(type, module, &cfg->users) ||
                false;
        }

        case 'V': {
            return
                tryModule(type, module, &cfg->vulkan) ||
                false;
        }

        case 'W': {
            return
                tryModule(type, module, &cfg->wallpaper) ||
                tryModule(type, module, &cfg->weather) ||
                tryModule(type, module, &cfg->wm) ||
                tryModule(type, module, &cfg->wifi) ||
                tryModule(type, module, &cfg->wmTheme) ||
                false;
        }

        default:
            return false;
    }
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
        case 'p': case 'P': {
            if (ffStrEqualsIgnCase(type, FF_PUBLICIP_MODULE_NAME))
            {
                if (module) ffParsePublicIpJsonObject(&cfg->publicIP, module);
                ffPreparePublicIp(&cfg->publicIP);
            }
            break;
        }
        case 'w': case 'W': {
            if (ffStrEqualsIgnCase(type, FF_WEATHER_MODULE_NAME))
            {
                if (module) ffParseWeatherJsonObject(&cfg->weather, module);
                ffPrepareWeather(&cfg->weather);
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
    }

    return NULL;
}

const char* ffParseGeneralJsonConfig(void)
{
    FFconfig* config = &instance.config;

    yyjson_val* const root = yyjson_doc_get_root(instance.state.configDoc);
    assert(root);

    if (!yyjson_is_obj(root))
        return "Invalid JSON config format. Root value must be an object";

    yyjson_val* object = yyjson_obj_get(root, "general");
    if (!object) return NULL;
    if (!yyjson_is_obj(object)) return "Property 'general' must be an object";

    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(object, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);

        if (ffStrEqualsIgnCase(key, "allowSlowOperations"))
            config->allowSlowOperations = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "thread") || ffStrEqualsIgnCase(key, "multithreading"))
            config->multithreading = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "stat"))
        {
            if ((config->stat = yyjson_get_bool(val)))
                config->showErrors = true;
        }
        else if (ffStrEqualsIgnCase(key, "escapeBedrock"))
            config->escapeBedrock = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "pipe"))
            config->pipe = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "processingTimeout"))
            config->processingTimeout = (int32_t) yyjson_get_int(val);

        #if defined(__linux__) || defined(__FreeBSD__)
        else if (ffStrEqualsIgnCase(key, "playerName"))
            ffStrbufSetS(&config->playerName, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "osFile"))
            ffStrbufSetS(&config->osFile, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "dsForceDrm"))
            config->dsForceDrm = yyjson_get_bool(val);
        #elif defined(_WIN32)
        else if (ffStrEqualsIgnCase(key, "wmiTimeout"))
            config->wmiTimeout = (int32_t) yyjson_get_int(val);
        #endif

        else
            return "Unknown general property";
    }

    return NULL;
}

const char* ffParseDisplayJsonConfig(void)
{
    FFconfig* config = &instance.config;

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

        if (ffStrEqualsIgnCase(key, "showErrors"))
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
            config->percentType = (uint32_t) yyjson_get_uint(val);
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

const char* ffParseLibraryJsonConfig(void)
{
    FFconfig* config = &instance.config;

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
