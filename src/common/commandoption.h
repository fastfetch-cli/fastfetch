#pragma once

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
