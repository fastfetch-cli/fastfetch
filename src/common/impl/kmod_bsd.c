#include "common/kmod.h"
#include <sys/param.h>
#include <sys/module.h>

bool ffKmodLoaded(const char* modName)
{
    return modfind(modName) >= 0;
}
