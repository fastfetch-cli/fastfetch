#pragma once

#include "common/argType.h"

typedef struct FFformatarg
{
    FFArgType type;
    const void* value;
    const char* name; // argument name, must start with an alphabet
} FFformatarg;

void ffFormatAppendFormatArg(FFstrbuf* buffer, const FFformatarg* formatarg);
void ffParseFormatString(FFstrbuf* buffer, const FFstrbuf* formatstr, uint32_t numArgs, const FFformatarg* arguments);
#define FF_PARSE_FORMAT_STRING_CHECKED(buffer, formatstr, arguments) \
    ffParseFormatString((buffer), (formatstr), sizeof(arguments) / sizeof(*arguments), (arguments));
