#pragma once

#include "fastfetch.h"

// Things only needed by fastfetch
typedef struct FFdata
{
    FFstrbuf structure;
    bool configLoaded;
} FFdata;

void ffPrepareCommandOption(FFdata* data);
void ffPrintCommandOption(FFdata* data, yyjson_mut_doc* jsonDoc);
void ffMigrateCommandOptionToJsonc(FFdata* data, yyjson_mut_doc* jsonDoc);
