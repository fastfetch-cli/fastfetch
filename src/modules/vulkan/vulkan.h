#pragma once

#include "fastfetch.h"
#include <yyjson.h>

#define FF_VULKAN_MODULE_NAME "Vulkan"

void ffPrintVulkan(FFinstance* instance, FFVulkanOptions* options);
void ffInitVulkanOptions(FFVulkanOptions* options);
bool ffParseVulkanCommandOptions(FFVulkanOptions* options, const char* key, const char* value);
void ffDestroyVulkanOptions(FFVulkanOptions* options);
void ffParseVulkanJsonObject(FFinstance* instance, yyjson_val* module);
