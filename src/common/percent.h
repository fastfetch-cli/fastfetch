#pragma once

#include "util/FFstrbuf.h"
#include "common/parsing.h"
#include "common/option.h"

enum FFPercentageTypeFlags
{
    FF_PERCENTAGE_TYPE_NUM_BIT = 1 << 0,
    FF_PERCENTAGE_TYPE_BAR_BIT = 1 << 1,
    FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT = 1 << 2,
    FF_PERCENTAGE_TYPE_NUM_COLOR_BIT = 1 << 3,
    FF_PERCENTAGE_TYPE_BAR_MONOCHROME_BIT = FF_PERCENTAGE_TYPE_NUM_COLOR_BIT,
};

// if (green <= yellow)
// [0, green]: print green
// (green, yellow]: print yellow
// (yellow, 100]: print red
//
// if (green > yellow)
// [green, 100]: print green
// [yellow, green): print yellow
// [0, yellow): print red

void ffPercentAppendBar(FFstrbuf* buffer, double percent, FFColorRangeConfig config, const FFModuleArgs* module);
void ffPercentAppendNum(FFstrbuf* buffer, double percent, FFColorRangeConfig config, bool parentheses, const FFModuleArgs* module);

typedef struct yyjson_val yyjson_val;
typedef struct yyjson_mut_doc yyjson_mut_doc;
typedef struct yyjson_mut_val yyjson_mut_val;
bool ffPercentParseCommandOptions(const char* key, const char* subkey, const char* value, FFColorRangeConfig* config);
bool ffPercentParseJsonObject(const char* key, yyjson_val* value, FFColorRangeConfig* config);
void ffPercentGenerateJsonConfig(yyjson_mut_doc* doc, yyjson_mut_val* module, FFColorRangeConfig defaultConfig, FFColorRangeConfig config);
