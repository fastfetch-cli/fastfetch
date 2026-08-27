#include "common/kmod.h"

bool ffKmodLoaded([[maybe_unused]] const char* modName) {
    return true; // Don't generate kernel module related errors
}
