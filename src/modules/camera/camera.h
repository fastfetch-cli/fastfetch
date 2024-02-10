#pragma once

#include "fastfetch.h"

#define FF_CAMERA_MODULE_NAME "Camera"

void ffPrintCamera(FFCameraOptions* options);
void ffInitCameraOptions(FFCameraOptions* options);
void ffDestroyCameraOptions(FFCameraOptions* options);
