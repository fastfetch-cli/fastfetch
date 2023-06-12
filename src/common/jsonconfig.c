#include "fastfetch.h"
#include "common/jsonconfig.h"
#include "common/printing.h"
#include "common/io/io.h"
#include "common/time.h"
#include "modules/modules.h"

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>

bool ffJsonConfigParseModuleArgs(const char* key, yyjson_val* val, FFModuleArgs* moduleArgs)
{
    if(strcasecmp(key, "key") == 0)
    {
        ffStrbufSetNS(&moduleArgs->key, (uint32_t) yyjson_get_len(val), yyjson_get_str(val));
        return true;
    }
    else if(strcasecmp(key, "format") == 0)
    {
        ffStrbufSetNS(&moduleArgs->outputFormat, (uint32_t) yyjson_get_len(val), yyjson_get_str(val));
        return true;
    }
    else if(strcasecmp(key, "error") == 0)
    {
        ffStrbufSetNS(&moduleArgs->errorFormat, (uint32_t) yyjson_get_len(val), yyjson_get_str(val));
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
            if (strcasecmp(strVal, pPair->key) == 0)
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

static inline bool tryModule(FFinstance* instance, const char* type, yyjson_val* module, const char* moduleName, void (*const f)(FFinstance *instance, yyjson_val *module))
{
    if (strcasecmp(type, moduleName) == 0)
    {
        f(instance, module);
        return true;
    }
    return false;
}

static bool parseModuleJsonObject(FFinstance* instance, const char* type, yyjson_val* module)
{
    switch (toupper(type[0]))
    {
        case 'B': {
            return
                tryModule(instance, type, module, FF_BATTERY_MODULE_NAME, ffParseBatteryJsonObject) ||
                tryModule(instance, type, module, FF_BIOS_MODULE_NAME, ffParseBiosJsonObject) ||
                tryModule(instance, type, module, FF_BLUETOOTH_MODULE_NAME, ffParseBluetoothJsonObject) ||
                tryModule(instance, type, module, FF_BOARD_MODULE_NAME, ffParseBoardJsonObject) ||
                tryModule(instance, type, module, FF_BREAK_MODULE_NAME, ffParseBreakJsonObject) ||
                tryModule(instance, type, module, FF_BRIGHTNESS_MODULE_NAME, ffParseBrightnessJsonObject) ||
                false;
        }

        case 'C': {
            return
                tryModule(instance, type, module, FF_CHASSIS_MODULE_NAME, ffParseChassisJsonObject) ||
                tryModule(instance, type, module, FF_CPU_MODULE_NAME, ffParseCPUJsonObject) ||
                tryModule(instance, type, module, FF_CPUUSAGE_MODULE_NAME, ffParseCPUUsageJsonObject) ||
                tryModule(instance, type, module, FF_COMMAND_MODULE_NAME, ffParseCommandJsonObject) ||
                tryModule(instance, type, module, FF_COLORS_MODULE_NAME, ffParseColorsJsonObject) ||
                tryModule(instance, type, module, FF_CURSOR_MODULE_NAME, ffParseCursorJsonObject) ||
                tryModule(instance, type, module, FF_CUSTOM_MODULE_NAME, ffParseCustomJsonObject) ||
                false;
        }

        case 'D': {
            return
                tryModule(instance, type, module, FF_DATETIME_MODULE_NAME, ffParseDateTimeJsonObject) ||
                tryModule(instance, type, module, FF_DISPLAY_MODULE_NAME, ffParseDisplayJsonObject) ||
                tryModule(instance, type, module, FF_DISK_MODULE_NAME, ffParseDiskJsonObject) ||
                tryModule(instance, type, module, FF_DE_MODULE_NAME, ffParseDEJsonObject) ||
                false;
        }

        case 'F': {
            return
                tryModule(instance, type, module, FF_FONT_MODULE_NAME, ffParseFontJsonObject) ||
                false;
        }

        case 'G': {
            return
                tryModule(instance, type, module, FF_GAMEPAD_MODULE_NAME, ffParseGamepadJsonObject) ||
                tryModule(instance, type, module, FF_GPU_MODULE_NAME, ffParseGPUJsonObject) ||
                false;
        }

        case 'H': {
            return
                tryModule(instance, type, module, FF_HOST_MODULE_NAME, ffParseHostJsonObject) ||
                false;
        }

        case 'I': {
            return
                tryModule(instance, type, module, FF_ICONS_MODULE_NAME, ffParseIconsJsonObject) ||
                false;
        }

        case 'K': {
            return
                tryModule(instance, type, module, FF_KERNEL_MODULE_NAME, ffParseKernelJsonObject) ||
                false;
        }

        case 'L': {
            return
                tryModule(instance, type, module, FF_LOCALE_MODULE_NAME, ffParseLocaleJsonObject) ||
                tryModule(instance, type, module, FF_LOCALIP_MODULE_NAME, ffParseLocalIpJsonObject) ||
                false;
        }

        case 'M': {
            return
                tryModule(instance, type, module, FF_MEDIA_MODULE_NAME, ffParseMediaJsonObject) ||
                tryModule(instance, type, module, FF_MEMORY_MODULE_NAME, ffParseMemoryJsonObject) ||
                false;
        }

        case 'O': {
            return
                tryModule(instance, type, module, FF_OPENCL_MODULE_NAME, ffParseOpenCLJsonObject) ||
                tryModule(instance, type, module, FF_OPENGL_MODULE_NAME, ffParseOpenGLJsonObject) ||
                tryModule(instance, type, module, FF_OS_MODULE_NAME, ffParseOSJsonObject) ||
                false;
        }

        case 'P': {
            return
                tryModule(instance, type, module, FF_PACKAGES_MODULE_NAME, ffParsePackagesJsonObject) ||
                tryModule(instance, type, module, FF_PLAYER_MODULE_NAME, ffParsePlayerJsonObject) ||
                tryModule(instance, type, module, FF_POWERADAPTER_MODULE_NAME, ffParsePowerAdapterJsonObject) ||
                tryModule(instance, type, module, FF_PROCESSES_MODULE_NAME, ffParseProcessesJsonObject) ||
                tryModule(instance, type, module, FF_PUBLICIP_MODULE_NAME, ffParsePublicIpJsonObject) ||
                false;
        }

        case 'S': {
            return
                tryModule(instance, type, module, FF_SEPARATOR_MODULE_NAME, ffParseSeparatorJsonObject) ||
                tryModule(instance, type, module, FF_SHELL_MODULE_NAME, ffParseShellJsonObject) ||
                tryModule(instance, type, module, FF_SOUND_MODULE_NAME, ffParseSoundJsonObject) ||
                tryModule(instance, type, module, FF_SWAP_MODULE_NAME, ffParseSwapJsonObject) ||
                false;
        }

        case 'T': {
            return
                tryModule(instance, type, module, FF_TERMINAL_MODULE_NAME, ffParseTerminalJsonObject) ||
                tryModule(instance, type, module, FF_TERMINALFONT_MODULE_NAME, ffParseTerminalFontJsonObject) ||
                tryModule(instance, type, module, FF_TITLE_MODULE_NAME, ffParseTitleJsonObject) ||
                tryModule(instance, type, module, FF_THEME_MODULE_NAME, ffParseThemeJsonObject) ||
                false;
        }

        case 'U': {
            return
                tryModule(instance, type, module, FF_UPTIME_MODULE_NAME, ffParseUptimeJsonObject) ||
                tryModule(instance, type, module, FF_USERS_MODULE_NAME, ffParseUsersJsonObject) ||
                false;
        }

        case 'V': {
            return
                tryModule(instance, type, module, FF_VULKAN_MODULE_NAME, ffParseVulkanJsonObject) ||
                false;
        }

        case 'W': {
            return
                tryModule(instance, type, module, FF_WALLPAPER_MODULE_NAME, ffParseWallpaperJsonObject) ||
                tryModule(instance, type, module, FF_WEATHER_MODULE_NAME, ffParseWeatherJsonObject) ||
                tryModule(instance, type, module, FF_WM_MODULE_NAME, ffParseWMJsonObject) ||
                tryModule(instance, type, module, FF_WIFI_MODULE_NAME, ffParseWifiJsonObject) ||
                tryModule(instance, type, module, FF_WMTHEME_MODULE_NAME, ffParseWMThemeJsonObject) ||
                false;
        }

        default:
            return false;
    }
}

static const char* printJsonConfig(FFinstance* instance)
{
    yyjson_val* const root = yyjson_doc_get_root(instance->state.configDoc);
    assert(root);

    if (!yyjson_is_obj(root))
        return "Invalid JSON config format. Root value must be an object";

    yyjson_val* modules = yyjson_obj_get(root, "modules");
    if (!modules) return "Property 'modules' is not found";
    if (!yyjson_is_arr(modules)) return "Property 'modules' must be an array of strings or objects";

    yyjson_val* item;
    size_t idx, max;
    yyjson_arr_foreach(modules, idx, max, item)
    {
        uint64_t ms = 0;
        if(__builtin_expect(instance->config.stat, false))
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

        if(!parseModuleJsonObject(instance, type, module))
            return "Unknown module type";

        if(__builtin_expect(instance->config.stat, false))
        {
            char str[32];
            int len = snprintf(str, sizeof str, "%" PRIu64 "ms", ffTimeGetTick() - ms);
            if(instance->config.pipe)
                puts(str);
            else
                printf("\033[s\033[1A\033[9999999C\033[%dD%s\033[u", len, str); // Save; Up 1; Right 9999999; Left <len>; Print <str>; Load
        }

        #if defined(_WIN32) && defined(FF_ENABLE_BUFFER)
            fflush(stdout);
        #endif
    }

    return NULL;
}

const char* ffParseGeneralJsonConfig(FFinstance* instance)
{
    FFconfig* config = &instance->config;

    yyjson_val* const root = yyjson_doc_get_root(instance->state.configDoc);
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

        if (strcasecmp(key, "allowSlowOperations") == 0)
            config->allowSlowOperations = yyjson_get_bool(val);
        else if (strcasecmp(key, "thread") == 0 || strcasecmp(key, "multithreading") == 0)
            config->multithreading = yyjson_get_bool(val);
        else if (strcasecmp(key, "stat") == 0)
            config->stat = yyjson_get_bool(val);
        else if (strcasecmp(key, "escapeBedrock") == 0)
            config->escapeBedrock = yyjson_get_bool(val);
        else if (strcasecmp(key, "pipe") == 0)
            config->pipe = yyjson_get_bool(val);
        
        #if defined(__linux__) || defined(__FreeBSD__)
        else if (strcasecmp(key, "playerName") == 0)
            ffStrbufSetS(&config->playerName, yyjson_get_str(val));
        #endif

        else
            return "Unknown general property";
    }

    return NULL;
}

const char* ffParseDisplayJsonConfig(FFinstance* instance)
{
    FFconfig* config = &instance->config;

    yyjson_val* const root = yyjson_doc_get_root(instance->state.configDoc);
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

        if (strcasecmp(key, "showErrors") == 0)
            config->showErrors = yyjson_get_bool(val);
        else if (strcasecmp(key, "disableLinewrap") == 0)
            config->disableLinewrap = yyjson_get_bool(val);
        else if (strcasecmp(key, "hideCursor") == 0)
            config->hideCursor = yyjson_get_bool(val);
        else if (strcasecmp(key, "separator") == 0)
            ffStrbufAppendS(&config->keyValueSeparator, yyjson_get_str(val));
        else if (strcasecmp(key, "color") == 0)
        {
            if (yyjson_is_str(val))
            {
                ffOptionParseColor(yyjson_get_str(val), &config->colorKeys);
                ffStrbufAppend(&config->colorTitle, &config->colorKeys);
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
        else if (strcasecmp(key, "binaryPrefix") == 0)
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
        else if (strcasecmp(key, "percentType") == 0)
            config->percentType = (uint32_t) yyjson_get_uint(val);
        else
            return "Unknown display property";
    }

    return NULL;
}

const char* ffParseLibraryJsonConfig(FFinstance* instance)
{
    FFconfig* config = &instance->config;

    yyjson_val* const root = yyjson_doc_get_root(instance->state.configDoc);
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

        if (strcasecmp(key, "pci") == 0)
            ffStrbufAppendS(&config->libPCI, yyjson_get_str(val));
        else if (strcasecmp(key, "vulkan") == 0)
            ffStrbufAppendS(&config->libVulkan, yyjson_get_str(val));
        else if (strcasecmp(key, "freetype") == 0)
            ffStrbufAppendS(&config->libfreetype, yyjson_get_str(val));
        else if (strcasecmp(key, "wayland") == 0)
            ffStrbufAppendS(&config->libWayland, yyjson_get_str(val));
        else if (strcasecmp(key, "xcbRandr") == 0)
            ffStrbufAppendS(&config->libXcbRandr, yyjson_get_str(val));
        else if (strcasecmp(key, "xcb") == 0)
            ffStrbufAppendS(&config->libXcb, yyjson_get_str(val));
        else if (strcasecmp(key, "Xrandr") == 0)
            ffStrbufAppendS(&config->libXrandr, yyjson_get_str(val));
        else if (strcasecmp(key, "X11") == 0)
            ffStrbufAppendS(&config->libX11, yyjson_get_str(val));
        else if (strcasecmp(key, "gio") == 0)
            ffStrbufAppendS(&config->libGIO, yyjson_get_str(val));
        else if (strcasecmp(key, "DConf") == 0)
            ffStrbufAppendS(&config->libDConf, yyjson_get_str(val));
        else if (strcasecmp(key, "dbus") == 0)
            ffStrbufAppendS(&config->libDBus, yyjson_get_str(val));
        else if (strcasecmp(key, "XFConf") == 0)
            ffStrbufAppendS(&config->libXFConf, yyjson_get_str(val));
        else if (strcasecmp(key, "sqlite") == 0 || strcasecmp(key, "sqlite3") == 0)
            ffStrbufAppendS(&config->libSQLite3, yyjson_get_str(val));
        else if (strcasecmp(key, "rpm") == 0)
            ffStrbufAppendS(&config->librpm, yyjson_get_str(val));
        else if (strcasecmp(key, "imagemagick") == 0)
            ffStrbufAppendS(&config->libImageMagick, yyjson_get_str(val));
        else if (strcasecmp(key, "z") == 0)
            ffStrbufAppendS(&config->libZ, yyjson_get_str(val));
        else if (strcasecmp(key, "chafa") == 0)
            ffStrbufAppendS(&config->libChafa, yyjson_get_str(val));
        else if (strcasecmp(key, "egl") == 0)
            ffStrbufAppendS(&config->libEGL, yyjson_get_str(val));
        else if (strcasecmp(key, "glx") == 0)
            ffStrbufAppendS(&config->libGLX, yyjson_get_str(val));
        else if (strcasecmp(key, "osmesa") == 0)
            ffStrbufAppendS(&config->libOSMesa, yyjson_get_str(val));
        else if (strcasecmp(key, "opencl") == 0)
            ffStrbufAppendS(&config->libOpenCL, yyjson_get_str(val));
        else if (strcasecmp(key, "wlanapi") == 0)
            ffStrbufAppendS(&config->libwlanapi, yyjson_get_str(val));
        else if (strcasecmp(key, "pulse") == 0)
            ffStrbufAppendS(&config->libPulse, yyjson_get_str(val));
        else if (strcasecmp(key, "nm") == 0)
            ffStrbufAppendS(&config->libnm, yyjson_get_str(val));
        else
            return "Unknown library property";
    }

    return NULL;
}

void ffPrintJsonConfig(FFinstance* instance)
{
    const char* error = printJsonConfig(instance);
    if (error)
        ffPrintErrorString(instance, "JsonConfig", 0, NULL, NULL, "%s", error);
}
