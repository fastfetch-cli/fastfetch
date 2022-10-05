#include "board.h"

void ffDetectBoard(FFBoardResult* board)
{
    ffStrbufInitS(&board->error, "Not supported on macOS");

    ffStrbufInit(&board->boardName);
    ffStrbufInit(&board->boardVendor);
    ffStrbufInit(&board->boardVersion);
}
