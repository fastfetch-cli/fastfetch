#include "board.h"
#include "util/windows/registry.h"
#include "util/smbiosHelper.h"

const char* ffDetectBoard(FFBoardResult* board)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;

    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, NULL))
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"HARDWARE\\DESCRIPTION\\System\\BIOS\") failed";

    if(!ffRegReadStrbuf(hKey, L"BaseBoardProduct", &board->name, NULL))
        return "ffRegReadStrbuf(hKey, L\"BaseBoardProduct\") failed";

    ffCleanUpSmbiosValue(&board->name);
    ffRegReadStrbuf(hKey, L"BaseBoardManufacturer", &board->vendor, NULL);
    ffCleanUpSmbiosValue(&board->vendor);
    ffRegReadStrbuf(hKey, L"BaseBoardVersion", &board->version, NULL);
    ffCleanUpSmbiosValue(&board->version);

    return NULL;
}
