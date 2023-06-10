#include "fastfetch.h"
#include "common/jsonconfig.h"

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
