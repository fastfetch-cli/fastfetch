#include "chassis.h"

void ffDetectChassis(FFChassisResult* result)
{
    ffStrbufInitS(&result->error, "Not supported on this platform");

    ffStrbufInit(&result->chassisType);
    ffStrbufInit(&result->chassisVendor);
    ffStrbufInit(&result->chassisVersion);
}
