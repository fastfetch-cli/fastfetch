#pragma once

#include "fastfetch.h"

#define FF_DISK_MODULE_NAME "Disk"

void ffPrintDisk(FFDiskOptions* options);
void ffInitDiskOptions(FFDiskOptions* options);
bool ffParseDiskCommandOptions(FFDiskOptions* options, const char* key, const char* value);
void ffDestroyDiskOptions(FFDiskOptions* options);
void ffParseDiskJsonObject(yyjson_val* module);
