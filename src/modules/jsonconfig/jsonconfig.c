#include "modules/jsonconfig/jsonconfig.h"
#include "common/jsonconfig.h"
#include "common/printing.h"

#ifdef FF_HAVE_JSONC

#include "common/io/io.h"
#include "common/json.h"
#include "modules/modules.h"

#include <assert.h>

static inline bool parseModuleJsonObject(FFinstance* instance, const char* type, json_object* module)
{
    return
        ffParseTitleJsonObject(instance, type, module) ||
        ffParseBatteryJsonObject(instance, type, module) ||
        ffParseBiosJsonObject(instance, type, module) ||
        ffParseBluetoothJsonObject(instance, type, module) ||
        ffParseBoardJsonObject(instance, type, module) ||
        ffParseBreakJsonObject(instance, type, module) ||
        ffParseBrightnessJsonObject(instance, type, module) ||
        ffParseCommandJsonObject(instance, type, module) ||
        ffParseDateTimeJsonObject(instance, type, module) ||
        ffParseDisplayJsonObject(instance, type, module) ||
        ffParseHostJsonObject(instance, type, module) ||
        ffParseKernelJsonObject(instance, type, module) ||
        ffParseOSJsonObject(instance, type, module) ||
        ffParseSeparatorJsonObject(instance, type, module) ||
        false;
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
            if (!type) return "module object must contain a type key";
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
