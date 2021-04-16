#include "fastfetch.h"

#define FF_RESOLUTION_MODULE_NAME "Resolution"
#define FF_RESOLUTION_NUM_FORMAT_ARGS 3

//modules/resolution.c
void ffPrintResolutionValue(FFinstance* instance, uint8_t moduleIndex, FFcache* cache, int width, int height, int refreshRate);

//modules/resolution_wayland.c
bool ffPrintResolutionWaylandBackend(FFinstance* instance);

//modules/resolution_x11.c
bool ffPrintResolutionXrandrBackend(FFinstance* instance);
void ffPrintResolutionX11Backend(FFinstance* instance);
