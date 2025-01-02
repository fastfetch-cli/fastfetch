#pragma once

#include "fastfetch.h"

const char* ffDetectWMPlugin(FFstrbuf* pluginName);
const char* ffDetectWMVersion(const FFstrbuf* wmName, FFstrbuf* result, FFWMOptions* options);
