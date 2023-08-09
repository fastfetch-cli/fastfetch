#pragma once

#ifndef FF_INCLUDED_EDID_HELPER_H
#define FF_INCLUDED_EDID_HELPER_H

#include <stdint.h>
#include "util/FFstrbuf.h"

void ffEdidGetName(const uint8_t edid[128], FFstrbuf* name);
void ffEdidGetPhycialResolution(const uint8_t edid[128], uint32_t* width, uint32_t* height);
void ffEdidGetPhycialSize(const uint8_t edid[128], uint32_t* width, uint32_t* height); // in mm

#endif
