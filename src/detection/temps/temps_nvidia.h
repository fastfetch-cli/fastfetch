#pragma once

#include <stdint.h>

const char* ffDetectNvidiaGpuTemp(double* temp, const char* pciBusId, uint32_t pciDeviceId, uint32_t pciSubSystemId);
