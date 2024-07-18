#include "fastfetch.h"

typedef struct FFMonitorResult
{
    FFstrbuf name;
    uint32_t width; // native / maximum resolution, in pixels
    uint32_t height; // native / maximum resolution, in pixels
    double refreshRate;// maximum refresh rate in native resolution, in Hz
    uint32_t physicalWidth; // in mm
    uint32_t physicalHeight; // in mm
    bool hdrCompatible;
    uint16_t manufactureYear;
    uint16_t manufactureWeek;
    uint32_t serial;
} FFMonitorResult;

const char* ffDetectMonitor(FFlist* results);
