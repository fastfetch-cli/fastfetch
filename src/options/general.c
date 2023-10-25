#include "fastfetch.h"
#include "util/stringUtils.h"
#include "options/general.h"

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

        if (ffStrEqualsIgnCase(key, "allowSlowOperations"))
            options->allowSlowOperations = yyjson_get_bool(val);
        else if (ffStrEqualsIgnCase(key, "thread") || ffStrEqualsIgnCase(key, "multithreading"))
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
            options->dsForceDrm = yyjson_get_bool(val);
        #elif defined(_WIN32)
        else if (ffStrEqualsIgnCase(key, "wmiTimeout"))
            options->wmiTimeout = (int32_t) yyjson_get_int(val);
        #endif

        else
            return "Unknown general property";
    }

    return NULL;
}

bool ffOptionsParseGeneralCommandLine(FFOptionsGeneral* options, const char* key, const char* value)
{
    if(ffStrEqualsIgnCase(key, "--allow-slow-operations"))
        options->allowSlowOperations = ffOptionParseBoolean(value);
    else if(ffStrEqualsIgnCase(key, "--thread") || ffStrEqualsIgnCase(key, "--multithreading"))
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
        options->dsForceDrm = ffOptionParseBoolean(value);
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
    options->allowSlowOperations = false;
    options->multithreading = true;

    #if defined(__linux__) || defined(__FreeBSD__)
    options->escapeBedrock = true;
    ffStrbufInit(&options->playerName);
    ffStrbufInit(&options->osFile);
    options->dsForceDrm = false;
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
