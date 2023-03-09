#include "fastfetch.h"
#include "common/config.h"

#ifdef FF_HAVE_JSONC

bool ffJsonConfigParseModuleArgs(JSONCData* data, const char* key, json_object* val, FFModuleArgs* moduleArgs)
{
    if(strcasecmp(key, "key") == 0)
    {
        ffStrbufSetNS(&moduleArgs->key, (uint32_t) data->ffjson_object_get_string_len(val), data->ffjson_object_get_string(val));
        return true;
    }
    else if(strcasecmp(key, "format") == 0)
    {
        ffStrbufSetNS(&moduleArgs->outputFormat, (uint32_t) data->ffjson_object_get_string_len(val), data->ffjson_object_get_string(val));
        return true;
    }
    else if(strcasecmp(key, "error") == 0)
    {
        ffStrbufSetNS(&moduleArgs->errorFormat, (uint32_t) data->ffjson_object_get_string_len(val), data->ffjson_object_get_string(val));
        return true;
    }
    return false;
}

#endif
