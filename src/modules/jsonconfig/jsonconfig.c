#include "modules/jsonconfig/jsonconfig.h"
#include "common/jsonconfig.h"
#include "common/printing.h"

#ifdef FF_HAVE_JSONC

#include "common/io/io.h"
#include "common/json.h"
#include "modules/modules.h"

#include <assert.h>
#include <ctype.h>

static inline bool tryModule(FFinstance* instance, const char* type, json_object* module, const char* moduleName, void (*const f)(FFinstance *instance, json_object *module))
{
    if (strcasecmp(type, moduleName) == 0)
    {
        if (module) json_object_object_del(module, "type"); // this line frees `type`
        f(instance, module);
        return true;
    }
    return false;
}

static bool parseModuleJsonObject(FFinstance* instance, const char* type, json_object* module)
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

        case 'O': {
            return
                tryModule(instance, type, module, FF_OS_MODULE_NAME, ffParseOSJsonObject) ||
                false;
        }

        case 'S': {
            return
                tryModule(instance, type, module, FF_SEPARATOR_MODULE_NAME, ffParseSeparatorJsonObject) ||
                tryModule(instance, type, module, FF_SOUND_MODULE_NAME, ffParseSoundJsonObject) ||
                false;
        }

        case 'T': {
            return
                tryModule(instance, type, module, FF_TITLE_MODULE_NAME, ffParseTitleJsonObject) ||
                false;
        }

        default:
            return false;
    }
}

static const char* parseModules(FFinstance* instance, json_object* modules)
{
    array_list* list = json_object_get_array(modules);
    if (!list) return "modules must be an array of strings or objects";

    for (size_t idx = 0; idx < list->length; ++idx)
    {
        json_object* module = (json_object*) list->array[idx];
        const char* type = NULL;
        if (json_object_is_type(module, json_type_string))
        {
            type = json_object_get_string(module);
            module = NULL;
        }
        else if (json_object_is_type(module, json_type_object))
        {
            json_object* object = json_object_object_get(module, "type");
            type = json_object_get_string(object);
            if (!type) return "module object must contain a \"type\" key ( case sensitive )";
            if (json_object_object_length(module) == 1) // contains only Property type
                module = NULL;
        }
        else
            return "modules must be an array of strings or objects";

        if(!parseModuleJsonObject(instance, type, module))
            return "Unknown module type";
    }

    return NULL;
}

static const char* printJsonConfig(FFinstance* instance)
{
    if (!ffJsonLoadLibrary(instance))
        return "Failed to load json-c library";

    FF_STRBUF_AUTO_DESTROY content;
    ffStrbufInit(&content);
    FF_LIST_FOR_EACH(FFstrbuf, filename, instance->state.platform.configDirs)
    {
        uint32_t oldLength = filename->length;
        ffStrbufAppendS(filename, "fastfetch/config.jsonc");
        bool success = ffAppendFileBuffer(filename->chars, &content);
        ffStrbufSubstrBefore(filename, oldLength);
        if (success) break;
    }

    json_object* __attribute__((__cleanup__(wrapJsoncFree))) root = json_tokener_parse(content.chars);
    if (!root)
        return "Failed to parse JSON config file";

    lh_table* rootObject = json_object_get_object(root);
    if (!rootObject)
        return "Invalid JSON config format. Root value must be an object";

    struct lh_entry* entry;
    lh_foreach(rootObject, entry)
    {
        const char* key = (const char *)lh_entry_k(entry);
        json_object* val = (json_object *)lh_entry_v(entry);

        if (strcmp(key, "modules") == 0)
        {
            const char* error = parseModules(instance, val);
            if (error) return error;
        }
        else
            return "Unknown JSON config key in root object";
    }

    return NULL;
}

#else

static const char* printJsonConfig(FF_MAYBE_UNUSED FFinstance* instance)
{
    return "Fastfetch was compiled without json-c support";
}

#endif

void ffPrintJsonConfig(FFinstance* instance)
{
    const char* error = printJsonConfig(instance);
    if (error)
        ffPrintErrorString(instance, "JsonConfig", 0, NULL, NULL, "%s", error);
}
