#pragma once

#ifndef FF_INCLUDED_common_format
#define FF_INCLUDED_common_format

typedef enum FFformatargtype
{
    FF_FORMAT_ARG_TYPE_NULL = 0,
    FF_FORMAT_ARG_TYPE_UINT,
    FF_FORMAT_ARG_TYPE_UINT16,
    FF_FORMAT_ARG_TYPE_UINT8,
    FF_FORMAT_ARG_TYPE_INT,
    FF_FORMAT_ARG_TYPE_STRING,
    FF_FORMAT_ARG_TYPE_STRBUF,
    FF_FORMAT_ARG_TYPE_FLOAT,
    FF_FORMAT_ARG_TYPE_DOUBLE,
    FF_FORMAT_ARG_TYPE_LIST,
    FF_FORMAT_ARG_TYPE_BOOL
} FFformatargtype;

typedef struct FFformatarg
{
    FFformatargtype type;
    const void* value;
} FFformatarg;

void ffFormatAppendFormatArg(FFstrbuf* buffer, const FFformatarg* formatarg);
void ffParseFormatString(FFstrbuf* buffer, const FFstrbuf* formatstr, uint32_t numArgs, const FFformatarg* arguments);

#define FF_FORMAT_ARG_VALUE_BOOL(xpr) ((xpr) ? (const void*) 1 : NULL)

#endif
