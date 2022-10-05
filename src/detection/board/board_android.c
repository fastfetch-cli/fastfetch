#include "board.h"

void ffDetectBoard(FFBoardResult* board)
{
    ffStrbufInitS(&board->error, "Not supported on Android");

    ffStrbufInit(&board->boardName);
    ffStrbufInit(&board->boardVendor);
    ffStrbufInit(&board->boardVersion);
}
