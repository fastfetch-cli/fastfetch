#include "board.h"
#include "util/windows/registry.h"

const char* ffDetectBoard(FFBoardResult* board)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;

    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, NULL))
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"HARDWARE\\DESCRIPTION\\System\\BIOS\") failed";

    if(!ffRegReadStrbuf(hKey, L"BaseBoardProduct", &board->boardName, NULL))
        return "ffRegReadStrbuf(hKey, L\"BaseBoardProduct\") failed";

    ffRegReadStrbuf(hKey, L"BaseBoardManufacturer", &board->boardVendor, NULL);
    ffRegReadStrbuf(hKey, L"BaseBoardVersion", &board->boardVersion, NULL);

    return NULL;
}
