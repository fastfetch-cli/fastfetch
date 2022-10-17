extern "C" {
#include "board.h"
}
#include "util/windows/wmi.hpp"

extern "C" void ffDetectBoard(FFBoardResult* board)
{
    ffStrbufInit(&board->error);

    ffStrbufInit(&board->boardName);
    ffStrbufInit(&board->boardVendor);
    ffStrbufInit(&board->boardVersion);

    FFWmiQuery query(L"SELECT Product, Version, Manufacturer FROM Win32_BaseBoard", &board->error);
    if(!query)
        return;

    if(FFWmiRecord record = query.next())
    {
        record.getString(L"Product", &board->boardName);
        record.getString(L"Manufacturer", &board->boardVendor);
        record.getString(L"Version", &board->boardVersion);
    }
    else
        ffStrbufInitS(&board->error, "No Wmi result returned");
}
