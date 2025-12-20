#include "fastfetch.h"
#include "common/jsonconfig.h"
#include "common/processing.h"
#include "options/general.h"
#include "util/stringUtils.h"

#include <unistd.h>

const char* ffOptionsParseGeneralJsonConfig(FFOptionsGeneral* options, yyjson_val* root)
{
    yyjson_val* object = yyjson_obj_get(root, "general");
    if (!object) return NULL;
    if (!yyjson_is_obj(object)) return "Property 'general' must be an object";

    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(object, idx, max, key, val)
    {
        if (unsafe_yyjson_equals_str(key, "thread"))
            options->multithreading = yyjson_get_bool(val);
        else if (unsafe_yyjson_equals_str(key, "processingTimeout"))
            options->processingTimeout = (int32_t) yyjson_get_int(val);
        else if (unsafe_yyjson_equals_str(key, "preRun"))
        {
            if (!yyjson_is_str(val))
                return "general.preRun must be a string";
            if (system(unsafe_yyjson_get_str(val)) < 0)
                return "Failed to execute preRun command";
        }
        else if (unsafe_yyjson_equals_str(key, "detectVersion"))
            options->detectVersion = yyjson_get_bool(val);

        #if defined(__linux__) || defined(__FreeBSD__) || defined(__sun) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(__GNU__)
        else if (unsafe_yyjson_equals_str(key, "playerName"))
            ffStrbufSetJsonVal(&options->playerName, val);
        else if (unsafe_yyjson_equals_str(key, "dsForceDrm"))
        {
            if (yyjson_is_str(val))
            {
                int value;
                const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                    { "sysfs-only", FF_DS_FORCE_DRM_TYPE_SYSFS_ONLY },
                    { "false", FF_DS_FORCE_DRM_TYPE_FALSE },
                    { "true", FF_DS_FORCE_DRM_TYPE_TRUE },
                    {},
                });
                if (error)
                    return "Invalid enum value of `dsForceDrm`";
                else
                    options->dsForceDrm = (FFDsForceDrmType) value;
            }
            else
                options->dsForceDrm = yyjson_get_bool(val) ? FF_DS_FORCE_DRM_TYPE_TRUE : FF_DS_FORCE_DRM_TYPE_FALSE;
        }
        #elif defined(_WIN32)
        else if (unsafe_yyjson_equals_str(key, "wmiTimeout"))
            options->wmiTimeout = (int32_t) yyjson_get_int(val);
        #endif

        else
            return "Unknown general property";
    }

    return NULL;
}

bool ffOptionsParseGeneralCommandLine(FFOptionsGeneral* options, const char* key, const char* value)
{
    if(ffStrEqualsIgnCase(key, "--thread") || ffStrEqualsIgnCase(key, "--multithreading"))
        options->multithreading = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--processing-timeout"))
        options->processingTimeout = ffOptionParseInt32(key, value);
    else if(ffStrEqualsIgnCase(key, "--detect-version"))
        options->detectVersion = ffOptionParseBoolean(value);

    #if defined(__linux__) || defined(__FreeBSD__) || defined(__sun) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(__GNU__)
    else if(ffStrEqualsIgnCase(key, "--player-name"))
        ffOptionParseString(key, value, &options->playerName);
    else if(ffStrEqualsIgnCase(key, "--ds-force-drm"))
    {
        if (ffOptionParseBoolean(value))
            options->dsForceDrm = FF_DS_FORCE_DRM_TYPE_TRUE;
        else if (ffStrEqualsIgnCase(value, "sysfs-only"))
            options->dsForceDrm = FF_DS_FORCE_DRM_TYPE_SYSFS_ONLY;
        else
            options->dsForceDrm = FF_DS_FORCE_DRM_TYPE_FALSE;
    }
    #elif defined(_WIN32)
    else if (ffStrEqualsIgnCase(key, "--wmi-timeout"))
        options->wmiTimeout = ffOptionParseInt32(key, value);
    #endif

    else
        return false;

    return true;
}

void ffOptionsInitGeneral(FFOptionsGeneral* options)
{
    options->processingTimeout = 5000;
    options->multithreading = true;
    options->detectVersion = true;

    #if defined(__linux__) || defined(__FreeBSD__) || defined(__sun) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(__GNU__)
    ffStrbufInit(&options->playerName);
    options->dsForceDrm = FF_DS_FORCE_DRM_TYPE_FALSE;
    #elif defined(_WIN32)
    options->wmiTimeout = 5000;
    #endif
}

void ffOptionsDestroyGeneral(FF_MAYBE_UNUSED FFOptionsGeneral* options)
{
    #if defined(__linux__) || defined(__FreeBSD__) || defined(__sun) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(__GNU__)
    ffStrbufDestroy(&options->playerName);
    #endif
}

void ffOptionsGenerateGeneralJsonConfig(FFOptionsGeneral* options, yyjson_mut_doc* doc)
{
    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, doc->root, "general");

    yyjson_mut_obj_add_bool(doc, obj, "thread", options->multithreading);

    yyjson_mut_obj_add_int(doc, obj, "processingTimeout", options->processingTimeout);

    yyjson_mut_obj_add_bool(doc, obj, "detectVersion", options->detectVersion);

    #if defined(__linux__) || defined(__FreeBSD__) || defined(__sun) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(__GNU__)

    yyjson_mut_obj_add_strbuf(doc, obj, "playerName", &options->playerName);

    switch (options->dsForceDrm)
    {
        case FF_DS_FORCE_DRM_TYPE_FALSE:
            yyjson_mut_obj_add_bool(doc, obj, "dsForceDrm", false);
            break;
        case FF_DS_FORCE_DRM_TYPE_SYSFS_ONLY:
            yyjson_mut_obj_add_str(doc, obj, "dsForceDrm", "sysfs-only");
            break;
        case FF_DS_FORCE_DRM_TYPE_TRUE:
            yyjson_mut_obj_add_bool(doc, obj, "dsForceDrm", true);
            break;
    }

    #elif defined(_WIN32)

    yyjson_mut_obj_add_int(doc, obj, "wmiTimeout", options->wmiTimeout);

    #endif
}
