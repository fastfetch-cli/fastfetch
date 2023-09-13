#pragma once

#include "fastfetch.h"

#define FF_VULKAN_MODULE_NAME "Vulkan"

void ffPrintVulkan(FFVulkanOptions* options);
void ffInitVulkanOptions(FFVulkanOptions* options);
bool ffParseVulkanCommandOptions(FFVulkanOptions* options, const char* key, const char* value);
void ffDestroyVulkanOptions(FFVulkanOptions* options);
void ffParseVulkanJsonObject(FFVulkanOptions* options, yyjson_val* module);
void ffGenerateVulkanJson(FFVulkanOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
