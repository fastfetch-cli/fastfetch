#include "board.h"
#include "common/sysctl.h"
#include "common/smbios.h"

const char* ffDetectBoard(FFBoardResult* board) {
    if (ffSysctlGetString("machdep.dmi.board-product", &board->name) == nullptr) {
        ffCleanUpSmbiosValue(&board->name);
    }
    if (ffSysctlGetString("machdep.dmi.board-version", &board->version) == nullptr) {
        ffCleanUpSmbiosValue(&board->version);
    }
    if (ffSysctlGetString("machdep.dmi.board-vendor", &board->vendor) == nullptr) {
        ffCleanUpSmbiosValue(&board->vendor);
    }
    if (ffSysctlGetString("machdep.dmi.board-serial", &board->serial) == nullptr) {
        ffCleanUpSmbiosValue(&board->serial);
    }

    return nullptr;
}
