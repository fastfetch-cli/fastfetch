#pragma once

#include "fastfetch.h"

#define FF_DISK_MODULE_NAME "Disk"

void ffPrintDisk(FFinstance* instance, FFDiskOptions* options);
void ffInitDiskOptions(FFDiskOptions* options);
bool ffParseDiskCommandOptions(FFDiskOptions* options, const char* key, const char* value);
void ffDestroyDiskOptions(FFDiskOptions* options);

#ifdef FF_HAVE_JSONC
#include "common/jsonconfig.h"
void ffParseDiskJsonObject(FFinstance* instance, json_object* module);
#endif
