#pragma once

#include "fastfetch.h"

#define FF_VULKAN_MODULE_NAME "Vulkan"

void ffPrintVulkan(FFVulkanOptions* options);
void ffInitVulkanOptions(FFVulkanOptions* options);
void ffDestroyVulkanOptions(FFVulkanOptions* options);
