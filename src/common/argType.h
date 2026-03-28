#pragma once

#include "common/FFstrbuf.h"

// For ffRegReadValue(s)
// If length is 0, data will be allocated with malloc() and must be freed by the caller
// Otherwise, data must point to a buffer of at least length bytes, and the actual length of the data will be written to length.
typedef struct FFArgBuffer
{
    void* data;
    uint32_t length;
} FFArgBuffer;

typedef enum __attribute__((__packed__)) FFArgType
{
    FF_ARG_TYPE_NULL = 0,
    FF_ARG_TYPE_UINT,
    FF_ARG_TYPE_UINT64,
    FF_ARG_TYPE_UINT16,
    FF_ARG_TYPE_UINT8,
    FF_ARG_TYPE_INT,
    FF_ARG_TYPE_STRING,
    FF_ARG_TYPE_STRBUF,
    FF_ARG_TYPE_FLOAT,
    FF_ARG_TYPE_DOUBLE,
    FF_ARG_TYPE_LIST,
    FF_ARG_TYPE_BOOL,
    FF_ARG_TYPE_BUFFER,
} FFArgType;

#define FF_ARG(variable, var_name) { _Generic((variable), \
        uint32_t: FF_ARG_TYPE_UINT, \
        uint64_t: FF_ARG_TYPE_UINT64, \
        uint16_t: FF_ARG_TYPE_UINT16, \
        uint8_t: FF_ARG_TYPE_UINT8, \
        int32_t: FF_ARG_TYPE_INT, \
        char*: FF_ARG_TYPE_STRING, \
        const char*: FF_ARG_TYPE_STRING, \
        FFstrbuf: FF_ARG_TYPE_STRBUF, \
        float: FF_ARG_TYPE_FLOAT, \
        double: FF_ARG_TYPE_DOUBLE, \
        FFlist: FF_ARG_TYPE_LIST, \
        bool: FF_ARG_TYPE_BOOL, \
        FFArgBuffer: FF_ARG_TYPE_BUFFER \
    ), _Generic((variable), char*: (variable), const char*: (variable), default: &(variable) ), (var_name) }
