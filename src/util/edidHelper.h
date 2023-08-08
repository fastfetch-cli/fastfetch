#pragma once

#ifndef FF_INCLUDED_EDID_HELPER_H
#define FF_INCLUDED_EDID_HELPER_H

#include <stdint.h>

void ffEdidGetNativeResolution(uint8_t edid[128], uint32_t* width, uint32_t* height);

#endif
