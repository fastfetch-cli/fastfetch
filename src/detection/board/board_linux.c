#include "board.h"
#include "common/io.h"

#include <stdlib.h>

static bool hostValueSet(FFstrbuf* value)
{
    return
        value->length > 0 &&
        ffStrbufStartsWithIgnCaseS(value, "To be filled") != true &&
        ffStrbufStartsWithIgnCaseS(value, "To be set") != true &&
        ffStrbufStartsWithIgnCaseS(value, "OEM") != true &&
        ffStrbufStartsWithIgnCaseS(value, "O.E.M.") != true &&
        ffStrbufIgnCaseCompS(value, "None") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Product") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Product Name") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Product Version") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Name") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Version") != 0 &&
        ffStrbufIgnCaseCompS(value, "Default string") != 0 &&
        ffStrbufIgnCaseCompS(value, "Undefined") != 0 &&
        ffStrbufIgnCaseCompS(value, "Not Specified") != 0 &&
        ffStrbufIgnCaseCompS(value, "Not Applicable") != 0 &&
        ffStrbufIgnCaseCompS(value, "INVALID") != 0 &&
        ffStrbufIgnCaseCompS(value, "Type1ProductConfigId") != 0 &&
        ffStrbufIgnCaseCompS(value, "All Series") != 0
    ;
}

static void getHostValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer)
{
    ffReadFileBuffer(devicesPath, buffer);
    if(hostValueSet(buffer))
        return;

    ffReadFileBuffer(classPath, buffer);
    if(hostValueSet(buffer))
        return;

    ffStrbufClear(buffer);
}

void ffDetectBoard(FFBoardResult* board)
{
    ffStrbufInit(&board->error);

    ffStrbufInit(&board->boardName);
    getHostValue("/sys/devices/virtual/dmi/id/board_name", "/sys/class/dmi/id/board_name", &board->boardName);

    ffStrbufInit(&board->boardVendor);
    getHostValue("/sys/devices/virtual/dmi/id/board_vendor", "/sys/class/dmi/id/board_vendor", &board->boardVendor);

    ffStrbufInit(&board->boardVersion);
    getHostValue("/sys/devices/virtual/dmi/id/board_version", "/sys/class/dmi/id/board_version", &board->boardVersion);
}
