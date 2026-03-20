#include "common/kmod.h"

bool ffKmodLoaded(FF_MAYBE_UNUSED const char* modName)
{
    return true; // Don't generate kernel module related errors
}
