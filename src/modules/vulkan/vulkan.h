#pragma once

#include "option.h"

#define FF_VULKAN_MODULE_NAME "Vulkan"

bool ffPrintVulkan(FFVulkanOptions* options);
void ffInitVulkanOptions(FFVulkanOptions* options);
void ffDestroyVulkanOptions(FFVulkanOptions* options);

extern FFModuleBaseInfo ffVulkanModuleInfo;
