#pragma once

#include "common/parsing.h"

void ffTempsAppendNum(double celsius, FFstrbuf* buffer, FFColorRangeConfig config);
bool ffTempsParseCommandOptions(const char* key, const char* subkey, const char* value, bool* useTemp, FFColorRangeConfig* config);
bool ffTempsParseJsonObject(const char* key, yyjson_val* value, bool* useTemp, FFColorRangeConfig* config);
void ffTempsGenerateJsonConfig(yyjson_mut_doc* doc, yyjson_mut_val* module, FF_MAYBE_UNUSED bool defaultTemp, FFColorRangeConfig defaultConfig, bool temp, FFColorRangeConfig config);
