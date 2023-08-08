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
    else if(ffStrEqualsIgnCase(key, "keyColor"))
    {
        ffOptionParseColor(yyjson_get_str(val), &moduleArgs->keyColor);
        return true;
    }
    else if(ffStrEqualsIgnCase(key, "format"))
    {
        ffStrbufSetNS(&moduleArgs->outputFormat, (uint32_t) yyjson_get_len(val), yyjson_get_str(val));
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

static inline bool tryModule(const char* type, yyjson_val* module, const char* moduleName, void (*const f)(yyjson_val *module))
{
    if (ffStrEqualsIgnCase(type, moduleName))
    {
        f(module);
        return true;
    }
    return false;
}

static bool parseModuleJsonObject(const char* type, yyjson_val* module)
{
    switch (toupper(type[0]))
    {
        case 'B': {
            return
                tryModule(type, module, FF_BATTERY_MODULE_NAME, ffParseBatteryJsonObject) ||
                tryModule(type, module, FF_BIOS_MODULE_NAME, ffParseBiosJsonObject) ||
                tryModule(type, module, FF_BLUETOOTH_MODULE_NAME, ffParseBluetoothJsonObject) ||
                tryModule(type, module, FF_BOARD_MODULE_NAME, ffParseBoardJsonObject) ||
                tryModule(type, module, FF_BREAK_MODULE_NAME, ffParseBreakJsonObject) ||
                tryModule(type, module, FF_BRIGHTNESS_MODULE_NAME, ffParseBrightnessJsonObject) ||
                false;
        }

        case 'C': {
            return
                tryModule(type, module, FF_CHASSIS_MODULE_NAME, ffParseChassisJsonObject) ||
                tryModule(type, module, FF_CPU_MODULE_NAME, ffParseCPUJsonObject) ||
                tryModule(type, module, FF_CPUUSAGE_MODULE_NAME, ffParseCPUUsageJsonObject) ||
                tryModule(type, module, FF_COMMAND_MODULE_NAME, ffParseCommandJsonObject) ||
                tryModule(type, module, FF_COLORS_MODULE_NAME, ffParseColorsJsonObject) ||
                tryModule(type, module, FF_CURSOR_MODULE_NAME, ffParseCursorJsonObject) ||
                tryModule(type, module, FF_CUSTOM_MODULE_NAME, ffParseCustomJsonObject) ||
                false;
        }

        case 'D': {
            return
                tryModule(type, module, FF_DATETIME_MODULE_NAME, ffParseDateTimeJsonObject) ||
                tryModule(type, module, FF_DISPLAY_MODULE_NAME, ffParseDisplayJsonObject) ||
                tryModule(type, module, FF_DISK_MODULE_NAME, ffParseDiskJsonObject) ||
                tryModule(type, module, FF_DE_MODULE_NAME, ffParseDEJsonObject) ||
                false;
        }

        case 'F': {
            return
                tryModule(type, module, FF_FONT_MODULE_NAME, ffParseFontJsonObject) ||
                false;
        }

        case 'G': {
            return
                tryModule(type, module, FF_GAMEPAD_MODULE_NAME, ffParseGamepadJsonObject) ||
                tryModule(type, module, FF_GPU_MODULE_NAME, ffParseGPUJsonObject) ||
                false;
        }

        case 'H': {
            return
                tryModule(type, module, FF_HOST_MODULE_NAME, ffParseHostJsonObject) ||
                false;
        }

        case 'I': {
            return
                tryModule(type, module, FF_ICONS_MODULE_NAME, ffParseIconsJsonObject) ||
                false;
        }

        case 'K': {
            return
                tryModule(type, module, FF_KERNEL_MODULE_NAME, ffParseKernelJsonObject) ||
                false;
        }

        case 'L': {
            return
                tryModule(type, module, FF_LM_MODULE_NAME, ffParseLMJsonObject) ||
                tryModule(type, module, FF_LOCALE_MODULE_NAME, ffParseLocaleJsonObject) ||
                tryModule(type, module, FF_LOCALIP_MODULE_NAME, ffParseLocalIpJsonObject) ||
                false;
        }

        case 'M': {
            return
                tryModule(type, module, FF_MEDIA_MODULE_NAME, ffParseMediaJsonObject) ||
                tryModule(type, module, FF_MEMORY_MODULE_NAME, ffParseMemoryJsonObject) ||
                false;
        }

        case 'N': {
            return
                tryModule(type, module, FF_NATIVERESOLUTION_MODULE_NAME, ffParseNativeResolutionJsonObject) ||
                false;
        }

        case 'O': {
            return
                tryModule(type, module, FF_OPENCL_MODULE_NAME, ffParseOpenCLJsonObject) ||
                tryModule(type, module, FF_OPENGL_MODULE_NAME, ffParseOpenGLJsonObject) ||
                tryModule(type, module, FF_OS_MODULE_NAME, ffParseOSJsonObject) ||
                false;
        }

        case 'P': {
            return
                tryModule(type, module, FF_PACKAGES_MODULE_NAME, ffParsePackagesJsonObject) ||
                tryModule(type, module, FF_PLAYER_MODULE_NAME, ffParsePlayerJsonObject) ||
                tryModule(type, module, FF_POWERADAPTER_MODULE_NAME, ffParsePowerAdapterJsonObject) ||
                tryModule(type, module, FF_PROCESSES_MODULE_NAME, ffParseProcessesJsonObject) ||
                tryModule(type, module, FF_PUBLICIP_MODULE_NAME, ffParsePublicIpJsonObject) ||
                false;
        }

        case 'S': {
            return
                tryModule(type, module, FF_SEPARATOR_MODULE_NAME, ffParseSeparatorJsonObject) ||
                tryModule(type, module, FF_SHELL_MODULE_NAME, ffParseShellJsonObject) ||
                tryModule(type, module, FF_SOUND_MODULE_NAME, ffParseSoundJsonObject) ||
                tryModule(type, module, FF_SWAP_MODULE_NAME, ffParseSwapJsonObject) ||
                false;
        }

        case 'T': {
            return
                tryModule(type, module, FF_TERMINAL_MODULE_NAME, ffParseTerminalJsonObject) ||
                tryModule(type, module, FF_TERMINALFONT_MODULE_NAME, ffParseTerminalFontJsonObject) ||
                tryModule(type, module, FF_TERMINALSIZE_MODULE_NAME, ffParseTerminalSizeJsonObject) ||
                tryModule(type, module, FF_TITLE_MODULE_NAME, ffParseTitleJsonObject) ||
                tryModule(type, module, FF_THEME_MODULE_NAME, ffParseThemeJsonObject) ||
                false;
        }

        case 'U': {
            return
                tryModule(type, module, FF_UPTIME_MODULE_NAME, ffParseUptimeJsonObject) ||
                tryModule(type, module, FF_USERS_MODULE_NAME, ffParseUsersJsonObject) ||
                false;
        }

        case 'V': {
            return
                tryModule(type, module, FF_VULKAN_MODULE_NAME, ffParseVulkanJsonObject) ||
                false;
        }

        case 'W': {
            return
                tryModule(type, module, FF_WALLPAPER_MODULE_NAME, ffParseWallpaperJsonObject) ||
                tryModule(type, module, FF_WEATHER_MODULE_NAME, ffParseWeatherJsonObject) ||
                tryModule(type, module, FF_WM_MODULE_NAME, ffParseWMJsonObject) ||
                tryModule(type, module, FF_WIFI_MODULE_NAME, ffParseWifiJsonObject) ||
                tryModule(type, module, FF_WMTHEME_MODULE_NAME, ffParseWMThemeJsonObject) ||
                false;
        }

        default:
            return false;
    }
}

static const char* printJsonConfig(void)
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
        if(__builtin_expect(instance.config.stat, false))
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

        if(!parseModuleJsonObject(type, module))
            return "Unknown module type";

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
        else if (ffStrEqualsIgnCase(key, "noBuffer"))
            config->noBuffer = yyjson_get_bool(val);
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

void ffPrintJsonConfig(void)
{
    const char* error = printJsonConfig();
    if (error)
        ffPrintErrorString("JsonConfig", 0, NULL, NULL, "%s", error);
}
