#pragma once

#include <stdint.h>
#include "util/FFstrbuf.h"

void ffEdidGetVendorAndModel(const uint8_t edid[128], FFstrbuf* result);
bool ffEdidGetName(const uint8_t edid[128], FFstrbuf* name);
void ffEdidGetPreferredResolutionAndRefreshRate(const uint8_t edid[128], uint32_t* width, uint32_t* height, double* refreshRate);
void ffEdidGetPhysicalResolution(const uint8_t edid[128], uint32_t* width, uint32_t* height);
void ffEdidGetPhysicalSize(const uint8_t edid[128], uint32_t* width, uint32_t* height); // in mm
void ffEdidGetSerialAndManufactureDate(const uint8_t edid[128], uint32_t* serial, uint16_t* year, uint16_t* week);
bool ffEdidGetHdrCompatible(const uint8_t* edid, uint32_t length);
