#pragma once

#include "common/parsing.h"
#include "common/option.h"

// `buffer` and `module` are dereferenced unconditionally; `module` carries the key/color formatting
[[gnu::nonnull(2, 4)]] void ffTempsAppendNum(double celsius, FFstrbuf* buffer, FFColorRangeConfig config, const FFModuleArgs* module);
bool ffTempsParseCommandOptions(const char* key, const char* subkey, const char* value, bool* useTemp, FFColorRangeConfig* config);
bool ffTempsParseJsonObject(yyjson_val* key, yyjson_val* value, bool* useTemp, FFColorRangeConfig* config);
[[gnu::nonnull(1, 2)]] void ffTempsGenerateJsonConfig(yyjson_mut_doc* doc, yyjson_mut_val* module, bool temp, FFColorRangeConfig config);
