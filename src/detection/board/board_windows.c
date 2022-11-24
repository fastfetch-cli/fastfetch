#include "board.h"
#include "util/windows/register.h"

void ffDetectBoard(FFBoardResult* board)
{
    ffStrbufInit(&board->error);

    ffStrbufInit(&board->boardName);
    ffStrbufInit(&board->boardVendor);
    ffStrbufInit(&board->boardVersion);

    FF_HKEY_AUTO_DESTROY hKey = NULL;

    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, &board->error))
        return;

    if(!ffRegReadStrbuf(hKey, L"BaseBoardProduct", &board->boardName, &board->error))
        return;
    ffRegReadStrbuf(hKey, L"BaseBoardManufacturer", &board->boardVendor, NULL);
    ffRegReadStrbuf(hKey, L"BaseBoardVersion", &board->boardVersion, NULL);
}
