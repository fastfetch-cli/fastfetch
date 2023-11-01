#pragma once

#ifndef FF_INCLUDED_common_commandoption
#define FF_INCLUDED_common_commandoption

#include "fastfetch.h"

typedef struct FFCustomValue
{
    bool printKey;
    FFstrbuf key;
    FFstrbuf value;
} FFCustomValue;

// Things only needed by fastfetch
typedef struct FFdata
{
    FFstrbuf structure;
    FFlist customValues; // List of FFCustomValue
    bool loadUserConfig;
} FFdata;

bool ffParseModuleOptions(const char* key, const char* value);
void ffPrepareCommandOption(FFdata* data);
void ffPrintCommandOption(FFdata* data, yyjson_mut_doc* jsonDoc);
void ffMigrateCommandOptionToJsonc(FFdata* data, yyjson_mut_doc* jsonDoc);

#endif
