#pragma once

#include "option.h"

bool ffPrintVulkan(FFVulkanOptions* options);
void ffInitVulkanOptions(FFVulkanOptions* options);
void ffDestroyVulkanOptions(FFVulkanOptions* options);

extern FFModuleBaseInfo ffVulkanModuleInfo;
