#include "board.h"
#include "util/settings.h"

const char* ffDetectBoard(FFBoardResult* board)
{
    if (!ffSettingsGetAndroidProperty("ro.product.board", &board->name))
        ffSettingsGetAndroidProperty("ro.board.platform", &board->name);
    return NULL;
}
