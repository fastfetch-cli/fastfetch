#include "kernel.h"

void ffDetectKernel(FFinstance* instance, FFKernelResult* result)
{
    ffStrbufInit(&result->error);
    ffStrbufInitS(&result->sysname, instance->state.utsname.sysname);
    ffStrbufInitS(&result->release, instance->state.utsname.release);
    ffStrbufInitS(&result->version, instance->state.utsname.version);
}
