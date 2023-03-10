#pragma once

#ifdef FF_HAVE_JSONC

#include <json-c/json.h>

#include "common/library.h"

typedef struct JSONCData
{
    FF_LIBRARY_SYMBOL(json_tokener_parse)
    FF_LIBRARY_SYMBOL(json_object_is_type)
    FF_LIBRARY_SYMBOL(json_object_get_array)
    FF_LIBRARY_SYMBOL(json_object_get_boolean)
    FF_LIBRARY_SYMBOL(json_object_get_double)
    FF_LIBRARY_SYMBOL(json_object_get_int)
    FF_LIBRARY_SYMBOL(json_object_get_string_len)
    FF_LIBRARY_SYMBOL(json_object_get_string)
    FF_LIBRARY_SYMBOL(json_object_get_object)
    FF_LIBRARY_SYMBOL(json_object_object_get)
    FF_LIBRARY_SYMBOL(json_object_put)

    json_object* root;
} JSONCData;

bool ffJsonConfigParseModuleArgs(JSONCData* data, const char* key, json_object* val, FFModuleArgs* moduleArgs);
const char* ffJsonConfigParseEnum(JSONCData* data, json_object* val, int* result, FFKeyValuePair pairs[]);

#endif
