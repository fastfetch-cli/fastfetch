#pragma once

#include "common/ffdata.h"

void ffPrepareCommandOption(FFdata* data);
void ffPrintCommandOption(FFdata* data);
void ffMigrateCommandOptionToJsonc(FFdata* data);
bool ffParseModuleOptions(const char* key, const char* value);
