#include "fastfetch.h"
#include "common/jsonconfig.h"
#include "options/general.h"
#include "util/stringUtils.h"

#include <unistd.h>

const char* ffOptionsParseGeneralJsonConfig(FFOptionsGeneral* options, yyjson_val* root)
{
    yyjson_val* object = yyjson_obj_get(root, "general");
    if (!object) return NULL;
    if (!yyjson_is_obj(object)) return "Property 'general' must be an object";

    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(object, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);

        if (ffStrEqualsIgnCase(key, "thread") || ffStrEqualsIgnCase(key, "multithreading"))
            options->multithreading = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "processingTimeout"))
            options->processingTimeout = (int32_t) yyjson_get_int(val);

        #if defined(__linux__) || defined(__FreeBSD__)
        else if (ffStrEqualsIgnCase(key, "escapeBedrock"))
            options->escapeBedrock = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "playerName"))
            ffStrbufSetS(&options->playerName, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "osFile"))
            ffStrbufSetS(&options->osFile, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "dsForceDrm"))
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
        else if (ffStrEqualsIgnCase(key, "wmiTimeout"))
            options->wmiTimeout = (int32_t) yyjson_get_int(val);
        #endif

        else if (ffStrEqualsIgnCase(key, "stat"))
            return "Property `general.stat` has been changed to `display.stat`";
        else if (ffStrEqualsIgnCase(key, "pipe"))
            return "Property `general.pipe` has been changed to `display.pipe`";
        else if (ffStrEqualsIgnCase(key, "allowSlowOperations"))
            return "Property `general.allowSlowOperations` has been obsoleted. See CHANGELOG for detail";

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

    #if defined(__linux__) || defined(__FreeBSD__)
    else if(ffStrEqualsIgnCase(key, "--escape-bedrock"))
        options->escapeBedrock = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--player-name"))
        ffOptionParseString(key, value, &options->playerName);
    else if (ffStrEqualsIgnCase(key, "--os-file"))
        ffOptionParseString(key, value, &options->osFile);
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
    options->processingTimeout = 1000;
    options->multithreading = true;

    #if defined(__linux__) || defined(__FreeBSD__)
    options->escapeBedrock = true;
    ffStrbufInit(&options->playerName);
    ffStrbufInit(&options->osFile);
    options->dsForceDrm = FF_DS_FORCE_DRM_TYPE_FALSE;
    #elif defined(_WIN32)
    options->wmiTimeout = 5000;
    #endif
}

void ffOptionsDestroyGeneral(FF_MAYBE_UNUSED FFOptionsGeneral* options)
{
    #if defined(__linux__) || defined(__FreeBSD__)
    ffStrbufDestroy(&options->playerName);
    ffStrbufDestroy(&options->osFile);
    #endif
}

void ffOptionsGenerateGeneralJsonConfig(FFOptionsGeneral* options, yyjson_mut_doc* doc)
{
    __attribute__((__cleanup__(ffOptionsDestroyGeneral))) FFOptionsGeneral defaultOptions;
    ffOptionsInitGeneral(&defaultOptions);

    yyjson_mut_val* obj = yyjson_mut_obj(doc);

    if (options->multithreading != defaultOptions.multithreading)
        yyjson_mut_obj_add_bool(doc, obj, "thread", options->multithreading);

    if (options->processingTimeout != defaultOptions.processingTimeout)
        yyjson_mut_obj_add_int(doc, obj, "processingTimeout", options->processingTimeout);

    #if defined(__linux__) || defined(__FreeBSD__)

    if (options->escapeBedrock != defaultOptions.escapeBedrock)
        yyjson_mut_obj_add_bool(doc, obj, "escapeBedrock", options->escapeBedrock);

    if (!ffStrbufEqual(&options->playerName, &defaultOptions.playerName))
        yyjson_mut_obj_add_strbuf(doc, obj, "playerName", &options->playerName);

    if (!ffStrbufEqual(&options->osFile, &defaultOptions.osFile))
        yyjson_mut_obj_add_strbuf(doc, obj, "osFile", &options->osFile);

    if (options->dsForceDrm != defaultOptions.dsForceDrm)
    {
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
    }

    #elif defined(_WIN32)

    if (options->wmiTimeout != defaultOptions.wmiTimeout)
        yyjson_mut_obj_add_int(doc, obj, "wmiTimeout", options->wmiTimeout);

    #endif

    if (yyjson_mut_obj_size(obj) > 0)
        yyjson_mut_obj_add_val(doc, doc->root, "general", obj);
}
