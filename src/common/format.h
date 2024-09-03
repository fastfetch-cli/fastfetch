#pragma once

#include "util/FFstrbuf.h"

typedef enum FFformatArgType
{
    FF_FORMAT_ARG_TYPE_NULL = 0,
    FF_FORMAT_ARG_TYPE_UINT,
    FF_FORMAT_ARG_TYPE_UINT64,
    FF_FORMAT_ARG_TYPE_UINT16,
    FF_FORMAT_ARG_TYPE_UINT8,
    FF_FORMAT_ARG_TYPE_INT,
    FF_FORMAT_ARG_TYPE_STRING,
    FF_FORMAT_ARG_TYPE_STRBUF,
    FF_FORMAT_ARG_TYPE_FLOAT,
    FF_FORMAT_ARG_TYPE_DOUBLE,
    FF_FORMAT_ARG_TYPE_LIST,
    FF_FORMAT_ARG_TYPE_BOOL
} FFformatArgType;

#define FF_FORMAT_ARG(variable, var_name) { _Generic((variable), \
        uint32_t: FF_FORMAT_ARG_TYPE_UINT, \
        uint64_t: FF_FORMAT_ARG_TYPE_UINT64, \
        uint16_t: FF_FORMAT_ARG_TYPE_UINT16, \
        uint8_t: FF_FORMAT_ARG_TYPE_UINT8, \
        int32_t: FF_FORMAT_ARG_TYPE_INT, \
        char*: FF_FORMAT_ARG_TYPE_STRING, \
        const char*: FF_FORMAT_ARG_TYPE_STRING, \
        FFstrbuf: FF_FORMAT_ARG_TYPE_STRBUF, \
        float: FF_FORMAT_ARG_TYPE_FLOAT, \
        double: FF_FORMAT_ARG_TYPE_DOUBLE, \
        FFlist: FF_FORMAT_ARG_TYPE_LIST, \
        bool: FF_FORMAT_ARG_TYPE_BOOL \
    ), _Generic((variable), char*: (variable), const char*: (variable), default: &(variable) ), (var_name) }

typedef struct FFformatarg
{
    FFformatArgType type;
    const void* value;
    const char* name; // argument name, must start with an alphabet
} FFformatarg;

void ffFormatAppendFormatArg(FFstrbuf* buffer, const FFformatarg* formatarg);
void ffParseFormatString(FFstrbuf* buffer, const FFstrbuf* formatstr, uint32_t numArgs, const FFformatarg* arguments);
#define FF_PARSE_FORMAT_STRING_CHECKED(buffer, formatstr, numArgs, arguments) do {\
    static_assert(sizeof(arguments) / sizeof(*(arguments)) == (numArgs), "Invalid number of format arguments");\
    ffParseFormatString((buffer), (formatstr), (numArgs), (arguments));\
} while (0)
