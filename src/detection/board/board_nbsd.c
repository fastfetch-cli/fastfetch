#include "board.h"
#include "common/sysctl.h"
#include "util/smbiosHelper.h"

const char* ffDetectBoard(FFBoardResult* board)
{
    if (ffSysctlGetString("machdep.dmi.board-product", &board->name) == NULL)
        ffCleanUpSmbiosValue(&board->name);
    if (ffSysctlGetString("machdep.dmi.board-version", &board->version) == NULL)
        ffCleanUpSmbiosValue(&board->version);
    if (ffSysctlGetString("machdep.dmi.board-vendor", &board->vendor) == NULL)
        ffCleanUpSmbiosValue(&board->vendor);

    return NULL;
}
