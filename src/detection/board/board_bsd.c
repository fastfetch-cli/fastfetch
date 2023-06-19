#include "board.h"
#include "common/settings.h"

const char* ffDetectBoard(FFBoardResult* board)
{
    ffSettingsGetFreeBSDKenv("smbios.planar.product", &board->boardName);
    ffSettingsGetFreeBSDKenv("smbios.planar.maker", &board->boardVendor);
    ffSettingsGetFreeBSDKenv("smbios.planar.version", &board->boardVersion);
    return NULL;
}
