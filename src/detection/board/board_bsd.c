#include "board.h"
#include "common/settings.h"
#include "util/smbiosHelper.h"

const char* ffDetectBoard(FFBoardResult* result)
{
    ffSettingsGetFreeBSDKenv("smbios.planar.product", &result->boardName);
    ffCleanUpSmbiosValue(&result->boardName);
    ffSettingsGetFreeBSDKenv("smbios.planar.maker", &result->boardVendor);
    ffCleanUpSmbiosValue(&result->boardVendor);
    ffSettingsGetFreeBSDKenv("smbios.planar.version", &result->boardVersion);
    ffCleanUpSmbiosValue(&result->boardVersion);
    return NULL;
}
