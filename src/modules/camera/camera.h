#pragma once

#include "option.h"

bool ffPrintCamera(FFCameraOptions* options);
void ffInitCameraOptions(FFCameraOptions* options);
void ffDestroyCameraOptions(FFCameraOptions* options);

extern FFModuleBaseInfo ffCameraModuleInfo;
