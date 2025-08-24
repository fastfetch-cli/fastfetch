#pragma once

#include "option.h"

#define FF_CAMERA_MODULE_NAME "Camera"

bool ffPrintCamera(FFCameraOptions* options);
void ffInitCameraOptions(FFCameraOptions* options);
void ffDestroyCameraOptions(FFCameraOptions* options);

extern FFModuleBaseInfo ffCameraModuleInfo;
