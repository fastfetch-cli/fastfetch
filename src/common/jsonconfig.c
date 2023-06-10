#include "fastfetch.h"
#include "common/jsonconfig.h"

#ifdef FF_HAVE_JSONC

bool ffJsonConfigParseModuleArgs(const char* key, json_object* val, FFModuleArgs* moduleArgs)
{
    if(strcasecmp(key, "key") == 0)
    {
        ffStrbufSetNS(&moduleArgs->key, (uint32_t) json_object_get_string_len(val), json_object_get_string(val));
        return true;
    }
    else if(strcasecmp(key, "format") == 0)
    {
        ffStrbufSetNS(&moduleArgs->outputFormat, (uint32_t) json_object_get_string_len(val), json_object_get_string(val));
        return true;
    }
    else if(strcasecmp(key, "error") == 0)
    {
        ffStrbufSetNS(&moduleArgs->errorFormat, (uint32_t) json_object_get_string_len(val), json_object_get_string(val));
        return true;
    }
    return false;
}

const char* ffJsonConfigParseEnum(json_object* val, int* result, FFKeyValuePair pairs[])
{
    if (json_object_is_type(val, json_type_int))
    {
        int intVal = json_object_get_int(val);

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
    else if (json_object_is_type(val, json_type_string))
    {
        const char* strVal = json_object_get_string(val);
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

#endif
