#include "libc.h"

const char* ffDetectLibc(FFLibcResult* result)
{
    result->name = "Unknown";
    result->version = NULL;
    return NULL;
}
