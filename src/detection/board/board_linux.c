#include "board.h"
#include "common/io/io.h"
#include "util/smbiosHelper.h"

#include <stdlib.h>

const char* ffDetectBoard(FFBoardResult* board)
{
    if (ffGetSmbiosValue("/sys/devices/virtual/dmi/id/board_name", "/sys/class/dmi/id/board_name", &board->name))
    {
        ffGetSmbiosValue("/sys/devices/virtual/dmi/id/board_serial", "/sys/class/dmi/id/board_serial", &board->serial);
        ffGetSmbiosValue("/sys/devices/virtual/dmi/id/board_vendor", "/sys/class/dmi/id/board_vendor", &board->vendor);
        ffGetSmbiosValue("/sys/devices/virtual/dmi/id/board_version", "/sys/class/dmi/id/board_version", &board->version);
    }
    else if (ffReadFileBuffer("/proc/device-tree/board", &board->name))
    {
        ffStrbufTrimRightSpace(&board->name);
    }
    else if (ffReadFileBuffer("/proc/device-tree/compatible", &board->vendor))
    {
        uint32_t comma = ffStrbufFirstIndexC(&board->vendor, ',');
        if (comma < board->vendor.length)
        {
            ffStrbufSetS(&board->name, board->vendor.chars + comma + 1);
            ffStrbufTrimRightSpace(&board->name);
            ffStrbufSubstrBefore(&board->vendor, comma);
        }
    }
    return NULL;
}
