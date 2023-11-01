#pragma once

#include "fastfetch.h"

#define FF_OPENCL_MODULE_NAME "OpenCL"

void ffPrintOpenCL(FFOpenCLOptions* options);
void ffInitOpenCLOptions(FFOpenCLOptions* options);
void ffDestroyOpenCLOptions(FFOpenCLOptions* options);
