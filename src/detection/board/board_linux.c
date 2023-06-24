#include "board.h"
#include "common/io/io.h"
#include "util/smbiosHelper.h"

#include <stdlib.h>

static void getSmbiosValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer)
{
    ffReadFileBuffer(devicesPath, buffer);
    if(ffIsSmbiosValueSet(buffer))
        return;

    ffReadFileBuffer(classPath, buffer);
    if(ffIsSmbiosValueSet(buffer))
        return;

    ffStrbufClear(buffer);
}

const char* ffDetectBoard(FFBoardResult* board)
{
    getSmbiosValue("/sys/devices/virtual/dmi/id/board_name", "/sys/class/dmi/id/board_name", &board->name);
    getSmbiosValue("/sys/devices/virtual/dmi/id/board_vendor", "/sys/class/dmi/id/board_vendor", &board->vendor);
    getSmbiosValue("/sys/devices/virtual/dmi/id/board_version", "/sys/class/dmi/id/board_version", &board->version);
    return NULL;
}
