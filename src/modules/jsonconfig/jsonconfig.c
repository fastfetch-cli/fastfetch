#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/io/io.h"
#include "modules/jsonconfig/jsonconfig.h"
#include "modules/modules.h"

#include <assert.h>
#include <ctype.h>

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

static inline void wrapYyjsonFree(yyjson_doc** doc)
{
    assert(doc);
    if (*doc)
        yyjson_doc_free(*doc);
}

static const char* printJsonConfig(FFinstance* instance)
{
    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    FF_LIST_FOR_EACH(FFstrbuf, filename, instance->state.platform.configDirs)
    {
        uint32_t oldLength = filename->length;
        ffStrbufAppendS(filename, "fastfetch/config.jsonc");
        bool success = ffAppendFileBuffer(filename->chars, &content);
        ffStrbufSubstrBefore(filename, oldLength);
        if (success) break;
    }

    yyjson_doc* __attribute__((__cleanup__(wrapYyjsonFree))) doc = yyjson_read_opts(content.chars, content.length, YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS | YYJSON_READ_ALLOW_INF_AND_NAN, NULL, NULL);
    if (!doc)
        return "Failed to parse JSON config file";

    yyjson_val* const root = yyjson_doc_get_root(doc);
    assert(root);

    if (!yyjson_is_obj(root))
        return "Invalid JSON config format. Root value must be an object";

    yyjson_val* modules = yyjson_obj_get(root, "modules");
    if (!modules) return "Property 'modules' is not found";
    if (!yyjson_is_arr(modules)) return "modules must be an array of strings or objects";

    yyjson_val* item;
    size_t idx, max;
    yyjson_arr_foreach(modules, idx, max, item)
    {
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
    }

    return NULL;
}

void ffPrintJsonConfig(FFinstance* instance)
{
    const char* error = printJsonConfig(instance);
    if (error)
        ffPrintErrorString(instance, "JsonConfig", 0, NULL, NULL, "%s", error);
}
