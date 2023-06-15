#include "board.h"
#include "common/settings.h"

void ffDetectBoard(FFBoardResult* board)
{
    ffStrbufInit(&board->boardName);
    ffStrbufInit(&board->boardVendor);
    ffStrbufInit(&board->boardVersion);
    ffSettingsGetFreeBSDKenv("smbios.planar.product", &board->boardName);
    ffSettingsGetFreeBSDKenv("smbios.planar.maker", &board->boardVendor);
    ffSettingsGetFreeBSDKenv("smbios.planar.version", &board->boardVersion);
}
