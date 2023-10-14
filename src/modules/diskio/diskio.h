#pragma once

#include "fastfetch.h"

#define FF_DISKIO_MODULE_NAME "DiskIO"

void ffPrepareDiskIO(FFDiskIOOptions* options);

void ffPrintDiskIO(FFDiskIOOptions* options);
void ffInitDiskIOOptions(FFDiskIOOptions* options);
bool ffParseDiskIOCommandOptions(FFDiskIOOptions* options, const char* key, const char* value);
void ffDestroyDiskIOOptions(FFDiskIOOptions* options);
void ffParseDiskIOJsonObject(FFDiskIOOptions* options, yyjson_val* module);
void ffGenerateDiskIOJson(FFDiskIOOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module);
void ffPrintDiskIOHelpFormat(void);
