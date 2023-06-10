#pragma once

#include "fastfetch.h"

#define FF_VULKAN_MODULE_NAME "Vulkan"

void ffPrintVulkan(FFinstance* instance, FFVulkanOptions* options);
void ffInitVulkanOptions(FFVulkanOptions* options);
bool ffParseVulkanCommandOptions(FFVulkanOptions* options, const char* key, const char* value);
void ffDestroyVulkanOptions(FFVulkanOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseVulkanJsonObject(FFinstance* instance, json_object* module);
#endif
