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

bool ffParseModuleCommand(const char* type);
bool ffParseModuleOptions(const char* key, const char* value);
void ffPrepareCommandOption(FFdata* data);
void ffPrintCommandOption(FFdata* data);
void ffMigrateCommandOptionToJsonc(FFdata* data);

#endif
